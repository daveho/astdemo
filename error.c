#include "util.h"
#include "error.h"

void error_on_node(struct Node *node, const char *msg) {
  struct SourceInfo node_pos = node_get_source_info(node);
  error_at_pos(node_pos, msg);
}

void error_at_pos(struct SourceInfo source_pos, const char *msg) {
  err_fatal("%s:%d:%d: Error: %s\n", source_pos.filename, source_pos.line, source_pos.col, msg);
}
