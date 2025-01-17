#include "pydocs.h"

#include <string>

#include "compare.h"
#include "debug.h"
#include "global_data.h"
#include "workspace_global_data.h"

namespace Python {
String group_generics_inout(const String& group) {
  const Index gr = global_data::WsvGroupMap.at(group);

  std::pair<std::vector<String>, std::vector<String>> outdocs;
  for (auto& method : global_data::md_data_raw) {
    if (std::any_of(method.GOutType().cbegin(),
                    method.GOutType().cend(),
                    Cmp::eq(gr)))
      outdocs.first.push_back(method.Name());
    if (std::any_of(method.GInType().cbegin(),
                    method.GInType().cend(),
                    Cmp::eq(gr)))
      outdocs.second.push_back(method.Name());
  }

  std::sort(outdocs.first.begin(), outdocs.first.end(), str_compare_nocase);
  std::sort(outdocs.second.begin(), outdocs.second.end(), str_compare_nocase);

  String out;

  if (outdocs.first.size()) {
    out += var_string("\nWorkspace methods that can generate ",
                      group,
                      "\n",
                      String(36 + group.size(), '-'),
                      "\n\n.. hlist::\n    :columns: ",
                      hlist_num_cols(outdocs.first),
                      "\n");
    for (auto& m : outdocs.first)
      out += var_string("\n    * :func:`~pyarts.workspace.Workspace.", m, '`');
  }
  out += '\n';

  if (outdocs.second.size()) {
    out += var_string("\nWorkspace methods that require ",
                      group,
                      "\n",
                      String(31 + group.size(), '-'),
                      "\n\n.. hlist::\n    :columns: ",
                      hlist_num_cols(outdocs.second),
                      "\n");
    for (auto& m : outdocs.second)
      out += var_string("\n    * :func:`~pyarts.workspace.Workspace.", m, '`');
  }
  out += '\n';

  return out;
}

String group_workspace_types(const String& group) {
  const Index gr = global_data::WsvGroupMap.at(group);

  std::vector<String> vars;
  for (auto& var: global_data::wsv_data) {
    if (gr == var.Group()) vars.push_back(var.Name());
  }

  std::sort(vars.begin(), vars.end(), str_compare_nocase);

  String out;
  if (vars.size()) {
    out += var_string("\nWorkspace variables of type ",
                      group,
                      "\n",
                      String(28 + group.size(), '-'),
                      "\n\n.. hlist::\n    :columns: ",
                      hlist_num_cols(vars),
                      "\n");
    for (auto& m : vars)
      out += var_string("\n    * :attr:`~pyarts.workspace.Workspace.", m, '`');
  }

  return out + "\n";
}
}  // namespace Python
