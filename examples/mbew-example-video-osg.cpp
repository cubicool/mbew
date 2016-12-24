#include "mbew.h"

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
	MBEWUpdateCallback(mbew_t* mbew, mbew_num_t width, mbew_num_t height):
	_mbew(mbew),
	_width(width),
	_height(height) {
	}

	virtual void update(osg::NodeVisitor* nv, osg::Drawable* drawable) {
		osg::Image* image = _getImage(drawable);

		if(!image) return;

		if(mbew_iterate(_mbew, MBEW_ITER_VIDEO_ONLY | MBEW_ITER_FORMAT_RGB | MBEW_ITER_SYNC)) {
			if(!mbew_iter_sync(_mbew, _time.elapsedTime_n())) return;

			image->setImage(
				_width,
				_height,
				1,
				GL_RGBA,
				GL_BGRA,
				GL_UNSIGNED_BYTE,
				static_cast<unsigned char*>(mbew_iter_video_rgb(_mbew)),
				osg::Image::NO_DELETE
			);

			image->dirty();
		}

		else {
			mbew_reset(_mbew);

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
	mbew_t* _mbew;
	mbew_num_t _width;
	mbew_num_t _height;

	osg::ElapsedTime _time;
};

int main(int argc, char** argv) {
	osgViewer::Viewer viewer;

	mbew_t* mbew = mbew_create(MBEW_SRC_FILE, argv[1]);
	mbew_status_t status;

	if((status = mbew_status(mbew))) {
		std::cout << "Error opening context: " << argv[1] << std::endl;
		std::cout << "Status was: " << mbew_string(MBEW_TYPE_STATUS, status) << std::endl;

		return 1;
	}

	if(!mbew_property(mbew, MBEW_PROP_VIDEO).b) {
		std::cout << "File contains no video." << std::endl;

		return 1;
	}

	mbew_num_t width = mbew_property(mbew, MBEW_PROP_VIDEO_WIDTH).num;
	mbew_num_t height = mbew_property(mbew, MBEW_PROP_VIDEO_HEIGHT).num;

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

	geom->setUpdateCallback(new MBEWUpdateCallback(mbew, width, height));

	geode->addDrawable(geom);

	viewer.setSceneData(geode);
	viewer.setUpViewInWindow(50, 50, width + 40, height + 40);
	viewer.addEventHandler(new osgViewer::StatsHandler());

	int r = viewer.run();

	mbew_destroy(mbew);

	return r;
}
