#include <stdbool.h>
#include <stdlib.h>
#include <glib.h>

enum rule_type {
  RULE_TYPE_PID,
  RULE_TYPE_PROG_NAME,
  RULE_TYPE_PORT
};

struct rule {
  int prio;
  int mark;
  enum rule_type rule_type;
  union {
    struct {
      int pid;
      bool inherit;
    } pid;
    struct {
      const char * regex;
    } prog_name;
    struct {
      int port;
    } port;
  } u;
};

static struct rules {
  int default_mark;
  GHashTable/* <int, struct rule *> */ * rules_by_id;
  GTree/* <struct rule *, void> */ * prog_name_rules_sorted_by_prio;
  GHashTable/* <int, struct rule *> */ * port_rules_by_port;
} * rules = NULL;

static gint compare_rules_by_prio(gconstpointer a, gconstpointer b) {
  return ((struct rule*)a)->prio - ((struct rule*)b)->prio;
}

void rules_init(void) {
  rules = malloc(sizeof (struct rules));
  rules->rules_by_id = g_hash_table_new(g_direct_hash, g_direct_equal);
  rules->prog_name_rules_sorted_by_prio = g_tree_new(compare_rules_by_prio);
  rules->port_rules_by_port = g_hash_table_new(g_direct_hash, g_direct_equal);
}

int set_pid_rule(int prio, int pid, bool inherit, int mark) {
  return 0;
}

void load_config(const char * filename) {
  GError * error = NULL;
  GKeyFile * config;
  config = g_key_file_new();
  if (!g_key_file_load_from_file(config, filename, G_KEY_FILE_NONE, &error)) {
    g_printerr("error loading config file %s: %s\n", filename, error->message);
    exit(1);
  }

  rules->default_mark = g_key_file_get_integer(config,
      "global", "default mark", &error);
  if (error) {
    g_printerr("error loading parameter \"default mark\" from [global] context: %s\n",
	error->message);
    exit(1);
  }
}
