#include <glib.h>
#include <gio/gio.h>
#include <stdbool.h>

static void on_bus_acquired(GDBusConnection * conn, const gchar * name,
    gpointer data) {
  g_print("Acquireed bus %s.\n", name);
}
static void on_name_acquired(GDBusConnection * conn, const gchar * name,
    gpointer data) {
  g_print("Acquired the name %s on the session bus.\n", name);
}

static gboolean on_timer(gpointer data) {
  g_print("Timer!\n");
  return true;
}

int main(int argc, char * argv[]) {
  GOptionContext * opt_context;
  GError * opt_error = NULL;
  gint opt_queue_num = 42;
  GOptionEntry opt_entries[] = {
    { "queue-num", 0, 0, G_OPTION_ARG_INT, &opt_queue_num,
      "IPTables NFQUEUE number 1<=N<=32767", "N" },
    { NULL },
  };

  g_type_init();

  opt_context = g_option_context_new(NULL);
  g_option_context_add_main_entries(opt_context, opt_entries, NULL);
  if (!g_option_context_parse(opt_context, &argc, &argv, &opt_error)) {
    g_printerr("Error parsing options: %s\n", opt_error->message);
    return 1;
  }
  if (argc != 1) {
    g_printerr("Invalid option: %s\n", argv[1]);
    return 1;
  }

  guint owner_id = g_bus_own_name(G_BUS_TYPE_SESSION, "uy.org.netlabs.ptd",
      G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired, on_name_acquired,
      NULL, NULL, NULL);
  g_timeout_add(1000, on_timer, NULL);
  g_main_loop_run(g_main_loop_new(NULL, false));
  g_bus_unown_name(owner_id);
  // TODO
  return 0;
}
