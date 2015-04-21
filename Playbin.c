#include <gst/gst.h>
#include <gtk/gtk.h>

struct Player_data
{
  GstElement *play;
};

static void funcion_quit (GtkButton * boton, gpointer data);
static void create_windows (struct Player_data *pdata);
static void play_cb (GtkButton * button, gpointer pdata);
static void pause_cb (GtkButton * button, gpointer pdata);
static void stop_cb (GtkButton * button, gpointer pdata);

static void
create_windows (struct Player_data *pdata)
{
  GtkWidget *window;
  GtkWidget *hbox;
  GtkWidget *play_button, *stop_button, *pause_button;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  play_button = gtk_button_new_from_stock (GTK_STOCK_MEDIA_PLAY);
  stop_button = gtk_button_new_from_stock (GTK_STOCK_MEDIA_STOP);
  pause_button = gtk_button_new_from_stock (GTK_STOCK_MEDIA_PAUSE);
  hbox = gtk_hbox_new (TRUE, 5);

  gtk_window_set_title ((GtkWindow *) window, "Chipi_Player");
  gtk_box_pack_start ((GtkBox *) hbox, stop_button, TRUE, TRUE, 2);
  gtk_box_pack_start ((GtkBox *) hbox, play_button, TRUE, TRUE, 2);
  gtk_box_pack_start ((GtkBox *) hbox, pause_button, TRUE, TRUE, 2);

  gtk_container_add ((GtkContainer *) window, hbox);

  g_signal_connect ((GObject *) play_button, "clicked",
      (GCallback) play_cb, (gpointer) pdata);
  g_signal_connect ((GObject *) stop_button, "clicked",
      (GCallback) stop_cb, (gpointer) pdata);
  g_signal_connect ((GObject *) pause_button, "clicked",
      (GCallback) pause_cb, (gpointer) pdata);
  g_signal_connect ((GObject *) window, "destroy",
      (GCallback) funcion_quit, (gpointer) pdata);

  gtk_widget_show_all (window);

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
my_bus_callback (GstBus *bus, GstMessage *message, gpointer data)
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
main (gint argc, gchar *argv[])
{
  GstBus *bus;
  struct Player_data pdata;

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

  gtk_main ();

  return 0;
}
