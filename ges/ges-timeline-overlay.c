/* GStreamer Editing Services
 * Copyright (C) 2009 Edward Hervey <edward.hervey@collabora.co.uk>
 *               2009 Nokia Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:ges-timeline-overlay
 * @short_description: An object for manipulating media files in a GESTimeline
 * 
 * Represents all the output treams from a particular uri. It is assumed that
 * the URI points to a file of some type.
 */

#include "ges-internal.h"
#include "ges-timeline-overlay.h"
#include "ges-timeline-source.h"
#include "ges-track-object.h"
#include "ges-track-video-title-source.h"
#include "ges-track-video-overlay.h"
#include <string.h>

G_DEFINE_TYPE (GESTimelineOverlay, ges_tl_overlay, GES_TYPE_TIMELINE_OBJECT);

#define DEFAULT_PROP_TEXT ""
#define DEFAULT_PROP_FONT_DESC DEFAULT_FONT_DESC
#define DEFAULT_PROP_VALIGNMENT DEFAULT_VALIGNMENT
#define DEFAULT_PROP_HALIGNMENT DEFAULT_HALIGNMENT
#define GES_TIMELINE_TITLE_SRC_VALIGN_TYPE (ges_timeline_overlay_valign_get_type())
#define GES_TIMELINE_TITLE_SRC_HALIGN_TYPE (ges_timeline_overlay_halign_get_type())

enum
{
  PROP_0,
  PROP_MUTE,
  PROP_TEXT,
  PROP_FONT_DESC,
  PROP_HALIGNMENT,
  PROP_VALIGNMENT,
};

static void ges_tl_overlay_set_mute (GESTimelineOverlay * self, gboolean mute);

static void
ges_tl_overlay_set_text (GESTimelineOverlay * self, const gchar * text);

static void
ges_tl_overlay_set_font_desc (GESTimelineOverlay * self, const gchar *
    font_desc);

static void
ges_tl_overlay_set_valign (GESTimelineOverlay * self,
    GESTrackVideoTitleSrcVAlign);

static void
ges_tl_overlay_set_halign (GESTimelineOverlay * self,
    GESTrackVideoTitleSrcHAlign);

static GESTrackObject
    * ges_tl_overlay_create_track_object (GESTimelineObject * obj,
    GESTrack * track);

static GType ges_timeline_overlay_valign_get_type (void);

static GType ges_timeline_overlay_halign_get_type (void);

static void
ges_tl_overlay_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GESTimelineOverlay *tfs = GES_TIMELINE_OVERLAY (object);

  switch (property_id) {
    case PROP_MUTE:
      g_value_set_boolean (value, tfs->mute);
      break;
    case PROP_TEXT:
      g_value_set_string (value, tfs->text);
      break;
    case PROP_FONT_DESC:
      g_value_set_string (value, tfs->font_desc);
      break;
    case PROP_HALIGNMENT:
      g_value_set_enum (value, tfs->halign);
      break;
    case PROP_VALIGNMENT:
      g_value_set_enum (value, tfs->valign);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
ges_tl_overlay_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GESTimelineOverlay *tfs = GES_TIMELINE_OVERLAY (object);

  switch (property_id) {
    case PROP_MUTE:
      ges_tl_overlay_set_mute (tfs, g_value_get_boolean (value));
      break;
    case PROP_TEXT:
      ges_tl_overlay_set_text (tfs, g_value_get_string (value));
      break;
    case PROP_FONT_DESC:
      ges_tl_overlay_set_font_desc (tfs, g_value_get_string (value));
      break;
    case PROP_HALIGNMENT:
      ges_tl_overlay_set_halign (tfs, g_value_get_enum (value));
      break;
    case PROP_VALIGNMENT:
      ges_tl_overlay_set_valign (tfs, g_value_get_enum (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
ges_tl_overlay_dispose (GObject * object)
{
  GESTimelineOverlay *self = GES_TIMELINE_OVERLAY (object);

  if (self->text)
    g_free (self->text);
  if (self->font_desc)
    g_free (self->font_desc);

  G_OBJECT_CLASS (ges_tl_overlay_parent_class)->dispose (object);
}

static void
ges_tl_overlay_finalize (GObject * object)
{
  G_OBJECT_CLASS (ges_tl_overlay_parent_class)->finalize (object);
}

static void
ges_tl_overlay_class_init (GESTimelineOverlayClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GESTimelineObjectClass *timobj_class = GES_TIMELINE_OBJECT_CLASS (klass);

  object_class->get_property = ges_tl_overlay_get_property;
  object_class->set_property = ges_tl_overlay_set_property;
  object_class->dispose = ges_tl_overlay_dispose;
  object_class->finalize = ges_tl_overlay_finalize;

  /**
   * GESTimelineOverlay:text
   *
   * The text to diplay
   */

  g_object_class_install_property (object_class, PROP_TEXT,
      g_param_spec_string ("text", "Text", "The text to display",
          DEFAULT_PROP_TEXT, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  /**
   * GESTimelineOverlay:font-dec
   *
   * Pango font description string
   */

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_FONT_DESC,
      g_param_spec_string ("font-desc", "font description",
          "Pango font description of font to be used for rendering. "
          "See documentation of pango_font_description_from_string "
          "for syntax.", DEFAULT_PROP_FONT_DESC,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GESTimelineOverlay:valignment
   *
   * Vertical alignent of the text
   */
  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_VALIGNMENT,
      g_param_spec_enum ("valignment", "vertical alignment",
          "Vertical alignment of the text", GES_TIMELINE_TITLE_SRC_VALIGN_TYPE,
          DEFAULT_PROP_VALIGNMENT, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  /**
   * GESTimelineOverlay:halignment
   *
   * Horizontal alignment of the text
   */
  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_HALIGNMENT,
      g_param_spec_enum ("halignment", "horizontal alignment",
          "Horizontal alignment of the text",
          GES_TIMELINE_TITLE_SRC_HALIGN_TYPE, DEFAULT_PROP_HALIGNMENT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  /**
   * GESTimelineOverlay:mute:
   *
   * Whether the sound will be played or not.
   */
  g_object_class_install_property (object_class, PROP_MUTE,
      g_param_spec_boolean ("mute", "Mute", "Mute audio track",
          FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  timobj_class->create_track_object = ges_tl_overlay_create_track_object;
  timobj_class->need_fill_track = FALSE;
}

static void
ges_tl_overlay_init (GESTimelineOverlay * self)
{
  GES_TIMELINE_OBJECT (self)->duration = 0;
  self->text = NULL;
  self->text = NULL;
  self->halign = DEFAULT_PROP_HALIGNMENT;
  self->valign = DEFAULT_PROP_VALIGNMENT;
}

static GType
ges_timeline_overlay_valign_get_type (void)
{
  static GType text_overlay_valign_type = 0;
  static gsize initialized = 0;
  static const GEnumValue text_overlay_valign[] = {
    {GES_TRACK_VIDEO_TITLE_SRC_VALIGN_BASELINE, "baseline", "baseline"},
    {GES_TRACK_VIDEO_TITLE_SRC_VALIGN_BOTTOM, "bottom", "bottom"},
    {GES_TRACK_VIDEO_TITLE_SRC_VALIGN_TOP, "top", "top"},
    {0, NULL, NULL},
  };

  if (g_once_init_enter (&initialized)) {
    text_overlay_valign_type =
        g_enum_register_static ("GESTimelineOverlayVAlign",
        text_overlay_valign);
    g_once_init_leave (&initialized, 1);
  }
  return text_overlay_valign_type;
}

static GType
ges_timeline_overlay_halign_get_type (void)
{
  static GType text_overlay_halign_type = 0;
  static gsize initialized = 0;
  static const GEnumValue text_overlay_halign[] = {
    {GES_TRACK_VIDEO_TITLE_SRC_HALIGN_LEFT, "left", "left"},
    {GES_TRACK_VIDEO_TITLE_SRC_HALIGN_CENTER, "center", "center"},
    {GES_TRACK_VIDEO_TITLE_SRC_HALIGN_RIGHT, "right", "right"},
    {0, NULL, NULL},
  };

  if (g_once_init_enter (&initialized)) {
    text_overlay_halign_type =
        g_enum_register_static ("GESTimelineOverlayHAlign",
        text_overlay_halign);
    g_once_init_leave (&initialized, 1);
  }
  return text_overlay_halign_type;
}

static void
ges_tl_overlay_set_text (GESTimelineOverlay * self, const gchar * text)
{
  //GList *tmp;
  //GESTimelineObject *object = (GESTimelineObject *) self;

  GST_DEBUG ("self:%p, text:%s", self, text);

  if (self->text)
    g_free (self->text);

  self->text = g_strdup (text);

#if 0
  for (tmp = object->trackobjects; tmp; tmp = tmp->next) {
    GESTrackObject *trackobject = (GESTrackObject *) tmp->data;

    if (trackobject->track->type == GES_TRACK_TYPE_VIDEO)
      ges_track_video_overlay_set_text (GES_TRACK_VIDEO_OVERLAY
          (trackobject), self->text);
  }
#endif
}

static void
ges_tl_overlay_set_font_desc (GESTimelineOverlay * self, const gchar *
    font_desc)
{
  //GList *tmp;
  //GESTimelineObject *object = (GESTimelineObject *) self;

  GST_DEBUG ("self:%p, font_desc:%s", self, font_desc);

  if (self->font_desc)
    g_free (self->font_desc);

  self->font_desc = g_strdup (font_desc);
#if 0
  for (tmp = object->trackobjects; tmp; tmp = tmp->next) {
    GESTrackObject *trackobject = (GESTrackObject *) tmp->data;

    if (trackobject->track->type == GES_TRACK_TYPE_VIDEO)
      ges_track_video_overlay_set_font_desc (GES_TRACK_VIDEO_OVERLAY
          (trackobject), self->font_desc);
  }
#endif
}

static void
ges_tl_overlay_set_halign (GESTimelineOverlay * self,
    GESTrackVideoTitleSrcHAlign halign)
{
  //GList *tmp;
  //GESTimelineObject *object = (GESTimelineObject *) self;

  GST_DEBUG ("self:%p, halign:%d", self, halign);

  self->halign = halign;

#if 0
  for (tmp = object->trackobjects; tmp; tmp = tmp->next) {
    GESTrackObject *trackobject = (GESTrackObject *) tmp->data;

    if (trackobject->track->type == GES_TRACK_TYPE_VIDEO)
      ges_track_video_overlay_set_halignment (GES_TRACK_VIDEO_OVERLAY
          (trackobject), self->halign);
  }
#endif
}

static void
ges_tl_overlay_set_valign (GESTimelineOverlay * self,
    GESTrackVideoTitleSrcVAlign valign)
{
  //GList *tmp;
  //GESTimelineObject *object = (GESTimelineObject *) self;

  GST_DEBUG ("self:%p, valign:%d", self, valign);

  self->valign = valign;
#if 0
  for (tmp = object->trackobjects; tmp; tmp = tmp->next) {
    GESTrackObject *trackobject = (GESTrackObject *) tmp->data;

    if (trackobject->track->type == GES_TRACK_TYPE_VIDEO)
      ges_track_video_overlay_set_valignment (GES_TRACK_VIDEO_OVERLAY
          (trackobject), self->valign);
  }
#endif
}

static void
ges_tl_overlay_set_mute (GESTimelineOverlay * self, gboolean mute)
{
  GList *tmp;
  GESTimelineObject *object = (GESTimelineObject *) self;

  GST_DEBUG ("self:%p, mute:%d", self, mute);

  self->mute = mute;

  /* Go over tracked objects, and update 'active' status on all audio objects */
  for (tmp = object->trackobjects; tmp; tmp = tmp->next) {
    GESTrackObject *trackobject = (GESTrackObject *) tmp->data;

    if (trackobject->track->type == GES_TRACK_TYPE_AUDIO)
      ges_track_object_set_active (trackobject, !mute);
  }
}

static GESTrackObject *
ges_tl_overlay_create_track_object (GESTimelineObject * obj, GESTrack * track)
{

  GESTimelineOverlay *tfs = (GESTimelineOverlay *) obj;
  GESTrackObject *res = NULL;

  GST_DEBUG ("Creating a GESTrackOverlay");

  if (track->type == GES_TRACK_TYPE_VIDEO) {
    res = (GESTrackObject *) ges_track_video_overlay_new ();
    GST_DEBUG ("Setting text property");
    ges_track_video_overlay_set_text ((GESTrackVideoOverlay *) res, tfs->text);
  }

  else {
    res = GES_TRACK_OBJECT (ges_track_overlay_new ());
  }

  return res;
}

/**
 * ges_timeline_titlesource_new:
 *
 * Creates a new #GESTimelineOverlay
 *
 * Returns: The newly created #GESTimelineOverlay, or NULL if there was an
 * error.
 */
GESTimelineOverlay *
ges_timeline_overlay_new (void)
{
  /* FIXME : Check for validity/existence of URI */
  return g_object_new (GES_TYPE_TIMELINE_OVERLAY, NULL);
}