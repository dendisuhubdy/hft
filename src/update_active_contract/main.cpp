#include <stdio.h>
#include <string.h>
#include <libconfig.h++>
#include <Python.h>
#include <string>
int main() {
  // 配置文件路径
  // std::string config_path = "/home/zsr/hft/config/backtest/backtest.config";
  std::string config_path = "/root/hft/config/backtest/backtest.config";
  // 调用pyhton模块路径
  // std::string module_path = "/home/zsr/quant";
  std::string module_path = "/root/quant/tools";
  // 调用python模块名
  std::string module_name = "get_main";
  // 调用python函数名
  std::string function_name = "get_main";

  libconfig::Config cfg;
  cfg.readFile(config_path.c_str());
  // 初始化
  Py_Initialize();
  // 切换python工作路径到调用模块所在目录
  std::string chdir_cmd = std::string("sys.path.append(\"") + module_path + "\")";
  PyRun_SimpleString("import sys");
  PyRun_SimpleString(chdir_cmd.c_str());
  // 加载模块
  PyObject* pModule = PyImport_Import(PyString_FromString(module_name.c_str()));
  if (!pModule) {
    printf("[ERROR] Python get module failed\n");
    return 0;
  }
  // 加载函数
  PyObject* pv = PyObject_GetAttrString(pModule, function_name.c_str());
  if (!pv || !PyCallable_Check(pv)) {
    printf("[ERROR] Python get function failed\n");
    return 0;
  }
  libconfig::Setting & strategies = cfg.lookup("strategy");
  for (int i = 0 ; i < strategies.getLength() ; i++) {
    // 传入参数
    PyObject* args = PyTuple_New(1);
    PyObject* arg1 = Py_BuildValue("s", strategies[i]["unique_name"].c_str());
    PyTuple_SetItem(args, 0, arg1);
    // 调用函数
    PyObject* pReturn = PyObject_CallObject(pv, args);
    std::string return_value_prt;
    if (pReturn != NULL) {
      // libconfig::Setting没有提供清空list的方法，所以删除重建pairs
      strategies[i].remove("pairs");
      libconfig::Setting &pairs_list = strategies[i].add("pairs", libconfig::Setting::TypeList);
      // 将python返回的List逐个元素放入libconfig::Setting的pairs list中
      for (int j = 0 ; j < PyList_Size(pReturn) ; j++) {
        char *contract = NULL;
        PyArg_Parse(PyList_GetItem(pReturn, j), "z", &contract);
        pairs_list.add(libconfig::Setting::TypeString) = contract;
        return_value_prt += (std::string(contract) + " ");
      }
    }
    printf("unique_name:%s,return pairs:%s\n", strategies[i]["unique_name"].c_str(), pReturn == NULL ? "NULL,keep the old pairs" : return_value_prt.c_str());
  }
  cfg.writeFile(config_path.c_str());
  printf("update active contract finished\n");
  return 1;
}
