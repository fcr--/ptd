#include <glib.h>

int main(int argc, char * argv[]) {
  GOptionContext * opt_context;
  GError * opt_error = NULL;
  gint opt_queue_num = 42;
  GOptionEntry opt_entries[] = {
    { "queue-num", 0, 0, G_OPTION_ARG_INT, &opt_queue_num,
      "IPTables NFQUEUE number 1<=N<=32767", "N" },
    { NULL },
  };
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

  // TODO
  return 0;
}
