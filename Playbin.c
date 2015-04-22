#include <gst/gst.h>
#include <gtk/gtk.h>

struct Player_data
{
  GstElement *play;
  GtkWidget *current_time_l;
  GtkWidget *end_time_l;
  GtkWidget *slider;
  gulong signal_slider;
  gint64 duration;
};

static void funcion_quit (GtkButton * boton, gpointer data);
static void create_windows (struct Player_data *pdata);
static void play_cb (GtkButton * button, gpointer pdata);
static void pause_cb (GtkButton * button, gpointer pdata);
static void stop_cb (GtkButton * button, gpointer pdata);
static void slider_cb (GtkButton * button, gpointer data);
static gboolean update_slider (struct Player_data *pdata);

static void
create_windows (struct Player_data *pdata)
{
  GtkWidget *window;
  GtkWidget *hboxT;
  GtkWidget *vbox;
  GtkWidget *hboxB;
  GtkWidget *slider;
  GtkWidget *current_time_l;
  GtkWidget *end_time_l;
  GtkAdjustment *adjustment;
  GtkWidget *play_button, *stop_button, *pause_button;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  play_button = gtk_button_new_from_stock (GTK_STOCK_MEDIA_PLAY);
  stop_button = gtk_button_new_from_stock (GTK_STOCK_MEDIA_STOP);
  pause_button = gtk_button_new_from_stock (GTK_STOCK_MEDIA_PAUSE);
  adjustment = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 100.0, 1.0, 5.0,
      0.0);
  slider = gtk_hscale_new (adjustment);
  gtk_scale_set_draw_value ((GtkScale *) slider, FALSE);
  current_time_l = gtk_label_new ("00:00");
  end_time_l = gtk_label_new ("00:00");

  hboxT = gtk_hbox_new (TRUE, 10);
  hboxB = gtk_hbox_new (TRUE, 10);
  vbox = gtk_vbox_new (TRUE, 10);

  gtk_window_set_title ((GtkWindow *) window, "Chipi_Player");
  gtk_box_pack_start ((GtkBox *) hboxT, stop_button, TRUE, TRUE, 2);
  gtk_box_pack_start ((GtkBox *) hboxT, play_button, TRUE, TRUE, 2);
  gtk_box_pack_start ((GtkBox *) hboxT, pause_button, TRUE, TRUE, 2);

  gtk_box_pack_start ((GtkBox *) hboxB, current_time_l, TRUE, TRUE, 2);
  gtk_box_pack_start ((GtkBox *) hboxB, slider, TRUE, TRUE, 2);
  gtk_box_pack_start ((GtkBox *) hboxB, end_time_l, TRUE, TRUE, 2);

  gtk_box_pack_start ((GtkBox *) vbox, hboxT, TRUE, TRUE, 2);
  gtk_box_pack_start ((GtkBox *) vbox, hboxB, TRUE, TRUE, 2);

  gtk_container_add ((GtkContainer *) window, vbox);

  g_signal_connect ((GObject *) play_button, "clicked",
      (GCallback) play_cb, (gpointer) pdata);
  g_signal_connect ((GObject *) stop_button, "clicked",
      (GCallback) stop_cb, (gpointer) pdata);
  g_signal_connect ((GObject *) pause_button, "clicked",
      (GCallback) pause_cb, (gpointer) pdata);
  pdata->signal_slider = g_signal_connect ((GObject *) slider, "value-changed",
      (GCallback) slider_cb, (gpointer) pdata);
  g_signal_connect ((GObject *) window, "destroy",
      (GCallback) funcion_quit, (gpointer) pdata);

  gtk_widget_show_all (window);

  pdata->current_time_l = current_time_l;
  pdata->end_time_l = end_time_l;
  pdata->slider = slider;
}

static void
play_cb (GtkButton * button, gpointer pdata)
{
  gst_element_set_state (((struct Player_data *) (pdata))->play,
      GST_STATE_PLAYING);
}

static void
pause_cb (GtkButton * button, gpointer pdata)
{
  gst_element_set_state (((struct Player_data *) (pdata))->play,
      GST_STATE_PAUSED);
}

static void
stop_cb (GtkButton * button, gpointer pdata)
{
  gst_element_set_state (((struct Player_data *) (pdata))->play,
      GST_STATE_READY);
  gtk_range_set_value ((GtkRange *) ((struct Player_data *) (pdata))->slider,
      0.0);
}

static void
slider_cb (GtkButton * button, gpointer data)
{
  struct Player_data *pdata = (struct Player_data *) data;
  gdouble position;

  position = gtk_range_get_value ((GtkRange *) pdata->slider);

  gst_element_seek_simple (pdata->play, GST_FORMAT_TIME,
      GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
      (gint64) (position * (gdouble) pdata->duration) / 100.0);
}

static gboolean
update_slider (struct Player_data *pdata)
{
  gint64 duration;
  gint64 position;
  gint min, sec;
  gchar end_time[6];
  gchar curr_time[6];

  // Duration

  if (pdata->duration == -1) {
    if (gst_element_query_duration (pdata->play, GST_FORMAT_TIME, &duration)) {
      sec = (gdouble) (duration) / 1e9 + 0.5;
      min = sec / 60;
      sec = sec % 60;
      sprintf (end_time, "%02d:%02d", min, sec);
      gtk_label_set_text ((GtkLabel *) pdata->end_time_l, end_time);

      pdata->duration = duration;
    }
  }
  // Current time

  if (gst_element_query_position (pdata->play, GST_FORMAT_TIME, &position)) {
    sec = (gdouble) (position) / 1e9 + 0.5;
    min = sec / 60;
    sec = sec % 60;
    sprintf (curr_time, "%02d:%02d", min, sec);
    gtk_label_set_text ((GtkLabel *) pdata->current_time_l, curr_time);

    g_signal_handler_block (pdata->slider, pdata->signal_slider);
    gtk_range_set_value ((GtkRange *) pdata->slider,
        ((double) position / (double) pdata->duration) * 100.0);
    g_signal_handler_unblock (pdata->slider, pdata->signal_slider);
  }

  return TRUE;
}

static void
funcion_quit (GtkButton * boton, gpointer data)
{
  struct Player_data *pdata = (struct Player_data *) data;

  gst_element_set_state (pdata->play, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pdata->play));

  gtk_main_quit ();
}

static gboolean
my_bus_callback (GstBus * bus, GstMessage * message, gpointer data)
{
  g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));

  if (GST_MESSAGE_TYPE (message) == GST_MESSAGE_ERROR) {
    GError *err;
    gchar *debug;

    gst_message_parse_error (message, &err, &debug);
    g_print ("Error: %s\n", err->message);
    g_error_free (err);
    g_free (debug);

    gtk_main_quit ();
  }

  return TRUE;
}

gint
main (gint argc, gchar * argv[])
{
  GstBus *bus;
  struct Player_data pdata;
  pdata.duration = -1;

  gtk_init (&argc, &argv);
  gst_init (&argc, &argv);

  // Create pipeline and assign an example uri
  pdata.play = gst_element_factory_make ("playbin", "play");
  g_object_set (G_OBJECT (pdata.play), "uri",
      "http://upload.wikimedia.org/wikipedia/en/0/04/Rayman_2_music_sample.ogg",
      NULL);

  create_windows (&pdata);

  // Create bus for error handling
  bus = gst_pipeline_get_bus (GST_PIPELINE (pdata.play));
  gst_bus_add_signal_watch (bus);
  g_signal_connect (G_OBJECT (bus), "message::error",
      (GCallback) my_bus_callback, NULL);
  gst_object_unref (bus);

  // Start playing
  gst_element_set_state (pdata.play, GST_STATE_PLAYING);

  // These function will call every second
  g_timeout_add_seconds (1, (GSourceFunc) update_slider, &pdata);

  gtk_main ();

  return 0;
}
