#include <assert.h>
#include <stdbool.h>
#include <wlr/util/log.h>
#include "background-image.h"
#include "cairo.h"

cairo_surface_t *load_background_image(const char *path) {
	cairo_surface_t *image;
#ifdef HAVE_GDK_PIXBUF
	GError *err = NULL;
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, &err);
	if (!pixbuf) {
		wlr_log(L_ERROR, "Failed to load background image.");
		return false;
	}
	image = gdk_cairo_image_surface_create_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);
#else
	image = cairo_image_surface_create_from_png(path);
#endif //HAVE_GDK_PIXBUF
	if (!image) {
		wlr_log(L_ERROR, "Failed to read background image.");
		return NULL;
	}
	if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS) {
		wlr_log(L_ERROR, "Failed to read background image: %s."
#ifndef HAVE_GDK_PIXBUF
				"\nSway was compiled without gdk_pixbuf support, so only"
				"\nPNG images can be loaded. This is the likely cause."
#endif //HAVE_GDK_PIXBUF
				, cairo_status_to_string(cairo_surface_status(image)));
		return NULL;
	}
	return image;
}

void render_background_image(cairo_t *cairo, cairo_surface_t *image,
		enum background_mode mode, int wwidth, int wheight) {
	double width = cairo_image_surface_get_width(image);
	double height = cairo_image_surface_get_height(image);

	switch (mode) {
	case BACKGROUND_MODE_STRETCH:
		cairo_scale(cairo, (double)wwidth / width, (double)wheight / height);
		cairo_set_source_surface(cairo, image, 0, 0);
		break;
	case BACKGROUND_MODE_FILL: {
		double window_ratio = (double)wwidth / wheight;
		double bg_ratio = width / height;

		if (window_ratio > bg_ratio) {
			double scale = (double)wwidth / width;
			cairo_scale(cairo, scale, scale);
			cairo_set_source_surface(cairo, image,
					0, (double)wheight / 2 / scale - height / 2);
		} else {
			double scale = (double)wheight / height;
			cairo_scale(cairo, scale, scale);
			cairo_set_source_surface(cairo, image,
					(double)wwidth / 2 / scale - width / 2, 0);
		}
		break;
	}
	case BACKGROUND_MODE_FIT: {
		double window_ratio = (double)wwidth / wheight;
		double bg_ratio = width / height;

		if (window_ratio > bg_ratio) {
			double scale = (double)wheight / height;
			cairo_scale(cairo, scale, scale);
			cairo_set_source_surface(cairo, image,
					(double)wwidth / 2 / scale - width / 2, 0);
		} else {
			double scale = (double)wwidth / width;
			cairo_scale(cairo, scale, scale);
			cairo_set_source_surface(cairo, image,
					0, (double)wheight / 2 / scale - height / 2);
		}
		break;
	}
	case BACKGROUND_MODE_CENTER:
		cairo_set_source_surface(cairo, image,
				(double)wwidth / 2 - width / 2,
				(double)wheight / 2 - height / 2);
		break;
	case BACKGROUND_MODE_TILE: {
		cairo_pattern_t *pattern = cairo_pattern_create_for_surface(image);
		cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
		cairo_set_source(cairo, pattern);
		break;
	}
	case BACKGROUND_MODE_SOLID_COLOR:
		assert(0);
		break;
	}
	cairo_paint(cairo);
}
