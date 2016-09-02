/**
 * This is a test executable program
 */

#include "MVAUtils/BDT.h"
#include <iostream>
#include <string>
#include <stdexcept>

#include <TMVA/Reader.h>
#include <TMVA/MethodBDT.h>

#include <TXMLEngine.h>


struct XmlVariableInfo {
  std::string expression;
  std::string label;
  std::string varType;
  std::string nodeName;
};

std::vector<XmlVariableInfo>
parseVariables(TXMLEngine *xml, XMLNodePointer_t node, const std::string& nodeName)
{
  std::vector<XmlVariableInfo> result;
  if (!xml || !node) return result;

  // loop over all children inside <Variables> or <Spectators>
  for (XMLNodePointer_t info_node = xml->GetChild(node); info_node != 0;
       info_node = xml->GetNext(info_node))
  {
    XMLAttrPointer_t attr = xml->GetFirstAttr(info_node);
    XmlVariableInfo o;
    // loop over the attributes of each child
    while (attr != 0)
    {
      TString name = xml->GetAttrName(attr);
      if (name == "Expression")
        o.expression = xml->GetAttrValue(attr);
      else if (name == "Label")
        o.label = xml->GetAttrValue(attr);
      else if (name == "Type")
        o.varType = xml->GetAttrValue(attr);
      attr = xml->GetNextAttr(attr);
    }
    //          ATH_MSG_DEBUG("Expression: " << expression << " Label: " << label << " varType: " << varType);
    o.nodeName = nodeName;
    result.push_back(o);
  }
  return result;
}

std::vector<XmlVariableInfo> parseXml(char *inFile)
{
   std::vector<XmlVariableInfo> result;

  TXMLEngine xml;
  auto xmldoc = xml.ParseFile(inFile);
  if (!xmldoc) {
    std::cerr <<" file not found: "  << inFile << std::endl;
    throw std::runtime_error("file not found");
  }
  auto mainnode = xml.DocGetRootElement(xmldoc);

  // loop to find <Variables> and <Spectators>
  auto node = xml.GetChild(mainnode);
  while (node) {
    std::string nodeName(xml.GetNodeName(node));
    //if (nodeName == "Variables" || nodeName == "Spectators") {
    if (nodeName == "Variables") {
      std::vector<XmlVariableInfo> r = parseVariables(&xml, node, nodeName);
      result.insert(result.end(), r.begin(), r.end());
    }
    node = xml.GetNext(node);
  }
  xml.FreeDoc(xmldoc);
  return result;
}

int main(int argc, char* argv[])
{
  if (3 != argc) {
    std::cerr << "Usage: " << argv[0] << " <MVA name> <input xml>" << std::endl;
    return 1;
  }

  auto mvaName = argv[1];
  auto inFile = argv[2];

  std::vector<XmlVariableInfo> variables = parseXml(inFile);

  std::vector<float> varVals(variables.size());

  TMVA::Reader reader;

  int i = 0;
  for (const auto& variable : variables) {

    // std::cout << variable.nodeName << std::endl;
    // std::cout << variable.expression << std::endl;
    // std::cout << variable.label << std::endl;
    // std::cout << variable.varType << std::endl;

    std::string varDefinition = variable.label;
    if (variable.label != variable.expression) {
      varDefinition = variable.label + std::string(" := ") + variable.expression;
    }
    reader.AddVariable(varDefinition, &varVals[i++]);
  }

  reader.BookMVA(mvaName, inFile);

  TMVA::MethodBDT* tbdt = dynamic_cast<TMVA::MethodBDT*>(reader.FindMVA(mvaName));
  // std::cout << "tbdt = " << tbdt << std::endl;

  if (tbdt) {
    MVAUtils::BDT bdt(tbdt);
    std::cout << "Built the tree" << std::endl;
    bdt.PrintForest();
  }
  return 0;
}
