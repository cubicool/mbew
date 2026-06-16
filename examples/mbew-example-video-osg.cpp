#include "mbew.hpp"

#include <osg/Math>
#include <osg/Geometry>
#include <osg/TextureRectangle>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>
#include <osg/GLExtensions>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

/* osg/GL only pulls in <GL/gl.h>; GL_MAP_PERSISTENT_BIT/GL_MAP_COHERENT_BIT (ARB_buffer_storage)
 * aren't declared there and need glext.h directly. */
#include <GL/glext.h>

#include <cstring>
#include <iostream>

static const char* VERT_SRC = R"(
#version 330
in vec4 osg_Vertex;
in vec4 osg_MultiTexCoord0;
uniform mat4 osg_ModelViewProjectionMatrix;
out vec2 texCoord;
void main() {
    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
    texCoord = osg_MultiTexCoord0.xy;
}
)";

static const char* FRAG_SRC = R"(
#version 330
in vec2 texCoord;
out vec4 fragColor;
uniform sampler2DRect tex;
void main() {
    fragColor = texture(tex, texCoord);
}
)";

static const int RING_CAPACITY = 3;

/* Uploads each decoded frame via a persistent-mapped PBO ring (3 slots, matching the
 * PBO[uploading]/PBO[GPU-consuming]/PBO[next-write] split) and DSA immutable texture storage,
 * instead of OSG's osg::Image dirty-flag path. GL calls require a current context, which
 * osg::RenderInfo guarantees during the draw traversal but osg::Drawable::UpdateCallback does
 * not--so this is a DrawCallback rather than an UpdateCallback. DrawCallback::drawImplementation
 * is const, hence the mutable GL/ring state below. */
class MBEWDrawCallback: public osg::Drawable::DrawCallback {
public:
	MBEWDrawCallback(mbew::Context m, mbew::num_t width, mbew::num_t height):
	_m(m),
	_width(width),
	_height(height) {
	}

	virtual void drawImplementation(osg::RenderInfo& renderInfo, const osg::Drawable* drawable) const {
		osg::GLExtensions* ext = osg::GLExtensions::Get(renderInfo.getContextID(), true);

		if(!_initialized && !_init(ext)) {
			OSG_WARN << "Failed to initialize!" << std::endl;

			return;
		}

		// See mbew-example-video-osg.cpp's sibling examples for the iterate()/sync() contract;
		// unchanged from the CPU-upload version this replaces.
		if(_m->iterate(mbew::Iterate::VIDEO | mbew::Iterate::RGB | mbew::Iterate::SYNC)) {
			if(_m->iter.sync(_time.elapsedTime_n())) _upload(ext);
		}

		else {
			_m->reset();

			_time.reset();
		}

		drawable->drawImplementation(renderInfo);
	}

protected:
	bool _init(osg::GLExtensions* ext) const {
		if(
			!ext->glBufferStorage ||
			!ext->glMapBufferRange ||
			!ext->glTextureStorage2D ||
			!ext->glFenceSync ||
			!ext->glClientWaitSync
		) {
			std::cerr << "Required GL4 buffer-storage/texture-storage/sync extensions are unavailable." << std::endl;

			return false;
		}

		mbew::num_t frameSize = _m->frame_size(mbew::Iterate::RGB);

		// glGenTextures() only reserves a name; one bind is required to fix its target before
		// glTextureStorage2D() (DSA) will accept it, since this OSG build has no glCreateTextures.
		glGenTextures(1, &_texture);
		glBindTexture(GL_TEXTURE_RECTANGLE, _texture);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		ext->glTextureStorage2D(_texture, 1, GL_RGBA8, _width, _height);

		for(int i = 0; i < RING_CAPACITY; i++) {
			glGenBuffers(1, &_pbo[i]);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _pbo[i]);

			ext->glBufferStorage(
				GL_PIXEL_UNPACK_BUFFER,
				frameSize,
				NULL,
				GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
			);

			_pboPtr[i] = ext->glMapBufferRange(
				GL_PIXEL_UNPACK_BUFFER,
				0,
				frameSize,
				GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT
			);
		}

		_initialized = true;

		return true;
	}

	void _upload(osg::GLExtensions* ext) const {
		mbew::num_t frameSize = _m->frame_size(mbew::Iterate::RGB);

		// Guard against overwriting a slot the GPU might still be reading from 3 frames ago.
		if(_fence[_ring]) ext->glClientWaitSync(_fence[_ring], 0, GL_TIMEOUT_IGNORED);

		memcpy(_pboPtr[_ring], _m->iter.rgb(), frameSize);

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _pbo[_ring]);
		glBindTexture(GL_TEXTURE_RECTANGLE, _texture);

		// Offset-as-pointer: reads from the bound GL_PIXEL_UNPACK_BUFFER instead of client memory.
		glTexSubImage2D(
			GL_TEXTURE_RECTANGLE,
			0,
			0,
			0,
			_width,
			_height,
			GL_BGRA,
			GL_UNSIGNED_BYTE,
			0
		);

		// GL_PIXEL_UNPACK_BUFFER is a single global binding, not scoped to this texture/unit--
		// OSG's own PixelDataBufferObject (src/osg/BufferObject.cpp) always rebinds it to 0 right
		// after use for exactly this reason. Leaving our PBO bound here corrupted every other
		// glTexSubImage2D-style upload for the rest of the frame (e.g. StatsHandler's font atlas
		// glyph upload silently read garbage from our mapped PBO instead of the real bitmap).
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		// Not paired with glDeleteSync(): OSG's GLExtensions doesn't expose it, and glDeleteSync
		// isn't guaranteed to be directly linkable outside that dispatch table. Each slot leaks
		// one GLsync per RING_CAPACITY frames--bounded and reclaimed at process exit, acceptable
		// for this example.
		_fence[_ring] = ext->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		_ring = (_ring + 1) % RING_CAPACITY;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_RECTANGLE, _texture);
	}

private:
	mbew::Context _m;
	mbew::num_t _width;
	mbew::num_t _height;

	mutable osg::ElapsedTime _time;
	mutable bool _initialized = false;
	mutable GLuint _texture = 0;
	mutable GLuint _pbo[RING_CAPACITY] = {};
	mutable void* _pboPtr[RING_CAPACITY] = {};
	mutable GLsync _fence[RING_CAPACITY] = {};
	mutable int _ring = 0;
};

int main(int argc, char** argv) {
	if(argc < 2) {
		std::cout << "Must specify WebM file." << std::endl;

		return 1;
	}

	mbew::Context m = mbew::create(argv[1]);

	if(!m->valid()) {
		std::cout << "Error opening '" << argv[1] << "'" << std::endl;
		std::cout << "Status was: " << mbew::string(m->status()) << std::endl;

		return 1;
	}

	if(!m->property(mbew::Property::VIDEO).b) {
		std::cout << "File contains no video." << std::endl;

		return 1;
	}

	mbew::num_t width = m->property(mbew::Property::VIDEO_WIDTH).num;
	mbew::num_t height = m->property(mbew::Property::VIDEO_HEIGHT).num;

	osg::Geode* geode = new osg::Geode();
	// TextureRectangle coords are unnormalized pixels, (0,0) at bottom-left. mbew's RGB
	// buffer is row-major top-down, so the vertical (b/t) swap below compensates for that
	// mismatch with GL's bottom-up texture convention. There's no equivalent need to swap
	// left/right; column order already agrees between the buffer and texture s coordinate.
	osg::Geometry* geom = osg::createTexturedQuadGeometry(
		osg::Vec3(0.0f, 0.0f, 0.0f),
		osg::Vec3(width, 0.0f, 0.0f),
		osg::Vec3(0.0f, 0.0f, height),
		0.0f,
		height,
		width,
		0.0f
	);

	osg::Program* program = new osg::Program();
	program->addShader(new osg::Shader(osg::Shader::VERTEX, VERT_SRC));
	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, FRAG_SRC));

	osg::StateSet* state = geom->getOrCreateStateSet();

	state->setAttributeAndModes(program);
	state->addUniform(new osg::Uniform("tex", 0));
	state->setMode(GL_BLEND, osg::StateAttribute::ON);
	state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	geom->setDrawCallback(new MBEWDrawCallback(m, width, height));

	geode->addDrawable(geom);

	osgViewer::Viewer viewer;

	viewer.setSceneData(geode);
	viewer.setUpViewInWindow(50, 50, width + 40, height + 40);
	viewer.addEventHandler(new osgViewer::StatsHandler());

	int r = viewer.run();

	return r;
}
