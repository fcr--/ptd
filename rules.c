#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <glib.h>
#include <stdint.h>

enum rule_type {
  RULE_TYPE_PID,
  RULE_TYPE_PROG_NAME,
  RULE_TYPE_PORT
};

static int id_counter = 0;

struct rule {
  int id;
  int prio;
  int mark;
  enum rule_type rule_type;
  union {
    struct {
      int pid;
      bool inherit;
      GArray * inherited_pids; // so pid_rules_by_pid can be cleaned
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
  GHashTable/* <int, struct rule *> */ * pid_rules_by_pid;
} * rules = NULL;

static gint compare_rules_by_prio(gconstpointer a, gconstpointer b) {
  gint cmp = ((struct rule*)a)->prio - ((struct rule*)b)->prio;
  return cmp ? cmp : ((intptr_t)a - (intptr_t)b);
}

void rules_init(void) {
  rules = malloc(sizeof (struct rules));
  rules->rules_by_id = g_hash_table_new(g_direct_hash, g_direct_equal);
  rules->prog_name_rules_sorted_by_prio = g_tree_new(compare_rules_by_prio);
  rules->port_rules_by_port = g_hash_table_new(g_direct_hash, g_direct_equal);
  rules->pid_rules_by_pid = g_hash_table_new(g_direct_hash, g_direct_equal);
}

static int get_new_id(void) {
  while (g_hash_table_contains(rules->rules_by_id, GINT_TO_POINTER(id_counter))) {
    id_counter++;
    if (id_counter < 0) id_counter = 0;
  }
  int id = id_counter++;
  if (id_counter < 0) id_counter = 0;
  return id;
}

void del_rule(int id) {
  // TODO
}

int set_pid_rule(int prio, int pid, bool inherit, int mark) {
  // TODO
  return -1;
}

int set_prog_name_rule(int prio, const char * prog_name_regex, int mark) {
  // TODO
  return -1;
}

int set_port_rule(int prio, int port, int mark) {
  // validate parameters:
  if (port <= 0 || port > 65535 || mark < 0 || mark > 255)
    return -1;
  // remove if it was already defined:
  struct rule * old = g_hash_table_lookup(rules->port_rules_by_port, GINT_TO_POINTER(port));
  if (old) del_rule(old->id);
  // construct new port rule:
  struct rule * rule = malloc(sizeof (struct rule));
  rule->id = get_new_id();
  rule->prio = prio;
  rule->mark = mark;
  rule->rule_type = RULE_TYPE_PORT;
  rule->u.port.port = port;
  // add rule to collections:
  g_hash_table_insert(rules->rules_by_id, GINT_TO_POINTER(rule->id), rule);
  g_hash_table_insert(rules->port_rules_by_port, GINT_TO_POINTER(port), rule);
  g_print("added port rule: prio=%d port=%d mark=%d\n", prio, port, mark);
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

  gsize groups_count;
  gchar ** groups = g_key_file_get_groups(config, &groups_count);
  for (int i = 0; i < groups_count; i++) {
    if (!strcmp(groups[i], "global"))
      continue;
    char * type = g_key_file_get_string(config, groups[i], "type", &error);
    if (error) {
      g_printerr("error loading mandatory parameter \"type\" from [%s] context: %s\n",
	  groups[i], error->message);
      exit(1);
    }
    int prio = g_key_file_get_integer(config, groups[i], "prio", &error);
    if (error) {
      g_printerr("error loading mandatory parameter \"prio\" from [%s] context: %s\n",
	  groups[i], error->message);
      exit(1);
    }
    int mark = g_key_file_get_integer(config, groups[i], "mark", &error);
    if (error) {
      g_printerr("error loading mandatory parameter \"mark\" from [%s] context: %s\n",
	  groups[i], error->message);
      exit(1);
    }
    g_print("group: %s, type: %s, prio: %d, mark: %d\n", groups[i], type, prio, mark);

    if (!strcasecmp(type, "port")) {
      int port = g_key_file_get_integer(config, groups[i], "port", &error);
      if (error) {
	g_printerr("error loading mandatory parameter \"port\" for \"port\" type from [%s] context: %s\n",
	    groups[i], error->message);
	exit(1);
      }
      set_port_rule(prio, port, mark);
    }
  }
  g_strfreev(groups);
}
