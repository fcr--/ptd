#include <glib.h>

int main(int argc, char * argv[]) {
  GOptionContext * opt_context;
  GError * opt_error = NULL;
  GOptionEntry opt_entries[] = {
    { NULL },
  };
  opt_context = g_option_context_new("tagging daemon");
  g_option_context_add_main_entries(opt_context, opt_entries, NULL);
  if (!g_option_context_parse(opt_context, &argc, &argv, &opt_error)) {
    g_printerr("Error parsing options: %s", opt_error->message);
    return 1;
  }

  // TODO
  return 0;
}
