#include "SoftwareGraph.hpp"
#include "HyperedgeYAML.hpp"

#include <fstream>
#include <sstream>

int main(void)
{
    // NOTE: Everything in here could be a constructor of a derived class!
    Software::Graph swgraph;

    // Some constants
    const unsigned int maxMergeInputs = 10;

    // Supported Languages
    auto cId = swgraph.instantiateFrom(swgraph.createLanguage("C"));
    auto vhdlId = swgraph.instantiateFrom(swgraph.createLanguage("VHDL"));

    // Only interface: reals
    auto realId = swgraph.createInterface("RealNumber");
    // In C: float32
    auto floatId = swgraph.createDatatype("float");
    // In VHDL: std_logic_vector(31 downto 0)
    auto stdvecId = swgraph.createDatatype("std_logic_vector(31 downto 0)");
    swgraph.represents(unite(floatId, stdvecId), realId);
    swgraph.expressedIn(floatId, cId);
    swgraph.expressedIn(stdvecId, vhdlId);

    // Input/Output classes
    auto inputClassId = swgraph.createInput(realId, "RealInput");
    auto outputClassId = swgraph.createOutput(realId, "RealOutput");

    // Build up the behavior graph components aka Nodes
    // 1-to-1
    Hyperedges id1, id2;
    auto pipeId = swgraph.createAlgorithm("PIPE");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateOutput(outputClassId, "y");
    swgraph.needs(pipeId, id1);
    swgraph.provides(pipeId, id2);

    auto divId = swgraph.createAlgorithm("DIVIDE");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateOutput(outputClassId, "y");
    swgraph.needs(divId, id1);
    swgraph.provides(divId, id2);

    auto sinId = swgraph.createAlgorithm("SIN");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateOutput(outputClassId, "y");
    swgraph.needs(sinId, id1);
    swgraph.provides(sinId, id2);

    auto cosId = swgraph.createAlgorithm("COS");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateOutput(outputClassId, "y");
    swgraph.needs(cosId, id1);
    swgraph.provides(cosId, id2);

    auto tanId = swgraph.createAlgorithm("TAN");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateOutput(outputClassId, "y");
    swgraph.needs(tanId, id1);
    swgraph.provides(tanId, id2);

    auto tanhId = swgraph.createAlgorithm("TANH");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateOutput(outputClassId, "y");
    swgraph.needs(tanhId, id1);
    swgraph.provides(tanhId, id2);

    auto acosId = swgraph.createAlgorithm("ACOS");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateOutput(outputClassId, "y");
    swgraph.needs(acosId, id1);
    swgraph.provides(acosId, id2);

    auto asinId = swgraph.createAlgorithm("ASIN");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateOutput(outputClassId, "y");
    swgraph.needs(asinId, id1);
    swgraph.provides(asinId, id2);

    auto atanId = swgraph.createAlgorithm("ATAN");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateOutput(outputClassId, "y");
    swgraph.needs(atanId, id1);
    swgraph.provides(atanId, id2);

    auto logId = swgraph.createAlgorithm("LOG");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateOutput(outputClassId, "y");
    swgraph.needs(logId, id1);
    swgraph.provides(logId, id2);

    auto expId = swgraph.createAlgorithm("EXP");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateOutput(outputClassId, "y");
    swgraph.needs(expId, id1);
    swgraph.provides(expId, id2);

    auto absId = swgraph.createAlgorithm("ABS");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateOutput(outputClassId, "y");
    swgraph.needs(absId, id1);
    swgraph.provides(absId, id2);

    auto sqrtId = swgraph.createAlgorithm("SQRT");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateOutput(outputClassId, "y");
    swgraph.needs(sqrtId, id1);
    swgraph.provides(sqrtId, id2);

    // 2-to-1
    Hyperedges id3;
    auto atan2Id = swgraph.createAlgorithm("ATAN2");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateInput(inputClassId, "y");
    id3 = swgraph.instantiateOutput(outputClassId, "z");
    swgraph.needs(atan2Id, unite(id1, id2));
    swgraph.provides(atan2Id, id3);

    auto powId = swgraph.createAlgorithm("POW");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateInput(inputClassId, "y");
    id3 = swgraph.instantiateOutput(outputClassId, "z");
    swgraph.needs(powId, unite(id1, id2));
    swgraph.provides(powId, id3);

    auto modId = swgraph.createAlgorithm("MOD");
    id1 = swgraph.instantiateInput(inputClassId, "x");
    id2 = swgraph.instantiateInput(inputClassId, "y");
    id3 = swgraph.instantiateOutput(outputClassId, "z");
    swgraph.needs(modId, unite(id1, id2));
    swgraph.provides(modId, id3);

    // 3-to-1
    Hyperedges id4;
    auto greaterId = swgraph.createAlgorithm(">0");
    id1 = swgraph.instantiateInput(inputClassId, "condition");
    id2 = swgraph.instantiateInput(inputClassId, "true");
    id3 = swgraph.instantiateInput(inputClassId, "false");
    id4 = swgraph.instantiateOutput(outputClassId, "result");
    swgraph.needs(greaterId, unite(unite(id1, id2), id3));
    swgraph.provides(greaterId, id4);

    auto equalId = swgraph.createAlgorithm("==0");
    id1 = swgraph.instantiateInput(inputClassId, "condition");
    id2 = swgraph.instantiateInput(inputClassId, "true");
    id3 = swgraph.instantiateInput(inputClassId, "false");
    id4 = swgraph.instantiateOutput(outputClassId, "result");
    swgraph.needs(equalId, unite(unite(id1, id2), id3));
    swgraph.provides(equalId, id4);

    // (UP-TO-N)-to-1 aka Merges
    // they also have "bias" and "defaultValue"
    Hyperedges inputIds;
    auto sumId = swgraph.createAlgorithm("SUM");
    inputIds=unite(inputIds,swgraph.instantiateInput(inputClassId, "defaultValue"));
    inputIds=unite(inputIds,swgraph.instantiateInput(inputClassId, "bias"));
    for (unsigned int i = 0; i < maxMergeInputs; ++i)
    {
        std::stringstream ss;
        ss << "input" << i;
        inputIds = unite(inputIds, swgraph.instantiateInput(inputClassId, ss.str()));
    }
    id1 = swgraph.instantiateOutput(outputClassId, "merged");
    swgraph.needs(sumId, inputIds);
    swgraph.provides(sumId, id1);

    inputIds.clear();
    auto prodId = swgraph.createAlgorithm("PRODUCT");
    inputIds=unite(inputIds,swgraph.instantiateInput(inputClassId, "defaultValue"));
    inputIds=unite(inputIds,swgraph.instantiateInput(inputClassId, "bias"));
    for (unsigned int i = 0; i < maxMergeInputs; ++i)
    {
        std::stringstream ss;
        ss << "input" << i;
        inputIds = unite(inputIds, swgraph.instantiateInput(inputClassId, ss.str()));
    }
    id1 = swgraph.instantiateOutput(outputClassId, "merged");
    swgraph.needs(prodId, inputIds);
    swgraph.provides(prodId, id1);

    inputIds.clear();
    auto minId = swgraph.createAlgorithm("MIN");
    inputIds=unite(inputIds,swgraph.instantiateInput(inputClassId, "defaultValue"));
    inputIds=unite(inputIds,swgraph.instantiateInput(inputClassId, "bias"));
    for (unsigned int i = 0; i < maxMergeInputs; ++i)
    {
        std::stringstream ss;
        ss << "input" << i;
        inputIds = unite(inputIds, swgraph.instantiateInput(inputClassId, ss.str()));
    }
    id1 = swgraph.instantiateOutput(outputClassId, "merged");
    swgraph.needs(minId, inputIds);
    swgraph.provides(minId, id1);

    inputIds.clear();
    auto maxId = swgraph.createAlgorithm("MAX");
    inputIds=unite(inputIds,swgraph.instantiateInput(inputClassId, "defaultValue"));
    inputIds=unite(inputIds,swgraph.instantiateInput(inputClassId, "bias"));
    for (unsigned int i = 0; i < maxMergeInputs; ++i)
    {
        std::stringstream ss;
        ss << "input" << i;
        inputIds = unite(inputIds, swgraph.instantiateInput(inputClassId, ss.str()));
    }
    id1 = swgraph.instantiateOutput(outputClassId, "merged");
    swgraph.needs(maxId, inputIds);
    swgraph.provides(maxId, id1);

    inputIds.clear();
    auto meanId = swgraph.createAlgorithm("MEAN");
    inputIds=unite(inputIds,swgraph.instantiateInput(inputClassId, "defaultValue"));
    inputIds=unite(inputIds,swgraph.instantiateInput(inputClassId, "bias"));
    for (unsigned int i = 0; i < maxMergeInputs; ++i)
    {
        std::stringstream ss;
        ss << "input" << i;
        inputIds = unite(inputIds, swgraph.instantiateInput(inputClassId, ss.str()));
    }
    id1 = swgraph.instantiateOutput(outputClassId, "merged");
    swgraph.needs(meanId, inputIds);
    swgraph.provides(meanId, id1);

    inputIds.clear();
    auto normId = swgraph.createAlgorithm("NORM");
    inputIds=unite(inputIds,swgraph.instantiateInput(inputClassId, "defaultValue"));
    inputIds=unite(inputIds,swgraph.instantiateInput(inputClassId, "bias"));
    for (unsigned int i = 0; i < maxMergeInputs; ++i)
    {
        std::stringstream ss;
        ss << "input" << i;
        inputIds = unite(inputIds, swgraph.instantiateInput(inputClassId, ss.str()));
    }
    id1 = swgraph.instantiateOutput(outputClassId, "merged");
    swgraph.needs(normId, inputIds);
    swgraph.provides(normId, id1);

    // NOTE: The final BG nodes could be made as containers. However, we then have to make versions for all possible merge combinations...

    // TODO: Subclass the Nodes/Merges for C/VHDL implementations

    // Store the setup in a YAML for inspection
    YAML::Node test;
    test = static_cast<Hypergraph*>(&swgraph);

    std::ofstream fout;
    fout.open("def_bg.yml");
    if(fout.good()) {
        fout << test;
    } else {
        std::cout << "FAILED\n";
    }
    fout.close();
}