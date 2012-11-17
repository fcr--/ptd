#include <glib.h>
#include <gio/gio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "rules.h"
#include "interface.h"

static void on_dbus_obj_destroy_notify(void * data) {
  g_print("On user_data_free_func called.\n");
}

void on_dbus_method_call(GDBusConnection *conn, const gchar * sender,
    const gchar * object_path,const gchar * interface_name,
    const gchar * method_name, GVariant * parameters,
    GDBusMethodInvocation * invocation, gpointer data) {
  g_print("sender=%s object_path=%s interface_name=%s method_name=%s",
      sender, object_path, interface_name, method_name);
}

static void on_bus_acquired(GDBusConnection * conn, const gchar * name,
    gpointer data) {
  g_print("Acquireed bus %s.\n", name);
  GError * error = NULL;
  GDBusInterfaceInfo * interface_info = g_dbus_node_info_new_for_xml(
      dbus_interface, &error)->interfaces[0];
  if (error) {
    g_printerr("Error loading dbus interface from xml: %s\n", error->message);
    exit(1);
  }
  if (!interface_info) {
    g_printerr("Error: no dbus interface was specified in xml.\n");
    exit(1);
  }
  g_dbus_connection_register_object(conn, "/rules", interface_info,
      &(GDBusInterfaceVTable){.method_call = on_dbus_method_call},
      NULL, on_dbus_obj_destroy_notify, &error);
  if (error) {
    g_printerr("Registering /rules object: %s\n", error->message);
    exit(1);
  }
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
  const char * opt_rules_config_file = "ptd-rules.ini";
  gint opt_queue_num = 42;
  GOptionEntry opt_entries[] = {
    { "queue-num", 0, 0, G_OPTION_ARG_INT, &opt_queue_num,
      "IPTables NFQUEUE number 1<=N<=32767", "N" },
    { "rules-config-file", 'c', 0, G_OPTION_ARG_STRING, &opt_rules_config_file,
      "Filename for ptd-rules.ini", "FILENAME" },
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

  rules_init();
  load_config(opt_rules_config_file);

  guint owner_id = g_bus_own_name(G_BUS_TYPE_SYSTEM, "uy.com.netlabs.ptd",
      G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired, on_name_acquired,
      NULL, NULL, NULL);
  g_timeout_add(1000, on_timer, NULL);
  g_main_loop_run(g_main_loop_new(NULL, false));
  g_bus_unown_name(owner_id);
  // TODO
  return 0;
}
