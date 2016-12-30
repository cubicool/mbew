#include "mbew.hpp"

#include <osg/Math>
#include <osg/Geometry>
#include <osg/TextureRectangle>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <iostream>

class MBEWUpdateCallback: public osg::Drawable::UpdateCallback {
public:
	MBEWUpdateCallback(mbew::Context m, mbew::num_t width, mbew::num_t height):
	_m(m),
	_width(width),
	_height(height) {
	}

	virtual void update(osg::NodeVisitor* nv, osg::Drawable* drawable) {
		osg::Image* image = _getImage(drawable);

		if(!image) return;

		// This call to iterate() will only yield video data (mbew::Iterate::VIDEO), in addition to
		// requiring the caller to update the internal time state (mbew::Iterate::SYNC). When the
		// SYNC flag is specified, the corresponding iter.sync() method should be called once per
		// iteration, accepting the amount of time--in nanoseconds--that has elapsed since begining
		// the iteration. If the elapsed time equals or exceeds the pending frames timestamp,
		// iter.sync() will return true, instructing the caller to FULLY process this iteration
		// before the frame is advanced.
		//
		// The iterate() method itself will return true until either an error condition is met
		// (which can be queried via valid()/status()) or a single, complete iteration has been
		// performed.
		if(_m->iterate(mbew::Iterate::VIDEO | mbew::Iterate::RGB | mbew::Iterate::SYNC)) {
			if(!_m->iter.sync(_time.elapsedTime_n())) return;

			image->setImage(
				_width,
				_height,
				1,
				GL_RGBA,
				GL_BGRA,
				GL_UNSIGNED_BYTE,
				_m->iter.rgb(),
				osg::Image::NO_DELETE
			);

			image->dirty();
		}

		// Once a full iteration completes, reset both the context and the timer.
		else {
			_m->reset();

			_time.reset();
		}
	}

protected:
	osg::Image* _getImage(osg::Drawable* drawable) {
		osg::StateSet* ss = drawable->getStateSet();

		if(!ss) return NULL;

		osg::StateAttribute* sa = ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE);

		if(!sa) return NULL;

		osg::Texture* tex = sa->asTexture();

		if(!tex) return NULL;

		osg::Image* image = dynamic_cast<osg::Image*>(tex->getImage(0));

		if(!image) return NULL;

		return image;
	}

private:
	mbew::Context _m;
	mbew::num_t _width;
	mbew::num_t _height;

	osg::ElapsedTime _time;
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

	osg::Image* image = new osg::Image();
	osg::Geode* geode = new osg::Geode();
	osg::TextureRectangle* texture = new osg::TextureRectangle();
	osg::Geometry* geom = osg::createTexturedQuadGeometry(
		osg::Vec3(0.0f, 0.0f, 0.0f),
		osg::Vec3(width, 0.0f, 0.0f),
		osg::Vec3(0.0f, 0.0f, height),
		width,
		height,
		0.0f,
		0.0f
	);

	texture->setImage(image);
	texture->setDataVariance(osg::Object::DYNAMIC);

	osg::StateSet* state = geom->getOrCreateStateSet();

	state->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON);
	state->setMode(GL_BLEND, osg::StateAttribute::ON);
	state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	geom->setUpdateCallback(new MBEWUpdateCallback(m, width, height));

	geode->addDrawable(geom);

	osgViewer::Viewer viewer;

	viewer.setSceneData(geode);
	viewer.setUpViewInWindow(50, 50, width + 40, height + 40);
	viewer.addEventHandler(new osgViewer::StatsHandler());

	int r = viewer.run();

	return r;
}
