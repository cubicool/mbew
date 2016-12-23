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
	MBEWUpdateCallback(mbew_t* mbew):
	_mbew(mbew),
	_iter(NULL) {
	}

	virtual void update(osg::NodeVisitor*, osg::Drawable* drawable) {
		osg::StateSet* ss = drawable->getStateSet();

		if(!ss) return;

		osg::StateAttribute* sa = ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE);

		if(!sa) return;

		osg::Texture* tex = sa->asTexture();

		if(!tex) return;

		osg::Image* image = dynamic_cast<osg::Image*>(tex->getImage(0));

		if(!image) return;

		if(!_iter) _time.reset();

		if((_iter = mbew_iterate(_mbew, _iter))) {
			if(mbew_iter_type(_iter) != MBEW_DATA_VIDEO) return;

			mbew_data_video_t* video = mbew_iter_video(_iter);
			// mbew_ns_t timestamp = mbew_iter_timestamp(_iter);

			if(_time.elapsedTime_m() >= 250) {
				image->setImage(
					video->width,
					video->height,
					1,
					GL_RGBA,
					GL_BGRA,
					GL_UNSIGNED_INT_8_8_8_8_REV,
					static_cast<unsigned char*>(video->data),
					osg::Image::NO_DELETE
				);

				image->dirty();

				_time.reset();
			}
		}
	}

private:
	mbew_t* _mbew;
	mbew_iter_t* _iter;

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
		width, height, 0.0f,
		0.0f
	);

	texture->setImage(image);
	texture->setDataVariance(osg::Object::DYNAMIC);

	osg::StateSet* state = geom->getOrCreateStateSet();

	state->setTextureAttributeAndModes(
		0,
		texture,
		osg::StateAttribute::ON
	);

	state->setMode(GL_BLEND, osg::StateAttribute::ON);
	state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	geom->setUpdateCallback(new MBEWUpdateCallback(mbew));

	geode->addDrawable(geom);

	viewer.setSceneData(geode);
	viewer.setUpViewInWindow(50, 50, width + 40, height + 40);
	viewer.addEventHandler(new osgViewer::StatsHandler());

	int r = viewer.run();

	mbew_destroy(mbew);

	return r;
}
