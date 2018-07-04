#include "BehaviorGraph.hpp"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <sstream>

namespace Behavior {

Graph::Graph()
{
    setupMetaModel();
}

Graph::Graph(const Hypergraph& base)
: Software::Graph(base)
{
    setupMetaModel();
}

Graph::~Graph()
{
}

void Graph::setupMetaModel()
{
    Hyperedges uid, inputIds;
    const unsigned maxMergeInputs = 10;
    // Three main superclasses of the behavior graph domain:
    // NODE -- EDGE -- MERGE -- NODE ... pattern
    createAlgorithm("Behavior::Graph::Node", "NODE");
    createAlgorithm("Behavior::Graph::Merge", "MERGE");
    createAlgorithm("Behavior::Graph::Edge", "EDGE");
    // Main interfaces:
    // One common interface type, two main input/output classes
    createInterface("Behavior::Graph::Interface", "IEEE754::binary32");
    uid = createInput("Behavior::Graph::Input", "INPUT");
    uid = unite(uid, createOutput("Behavior::Graph::Output", "OUTPUT"));
    isA(uid, Hyperedges{"Behavior::Graph::Interface"});
    // Datatypes:
    // In C: float
    // In VHDL: 32 Bit std_logic_vector
    uid = createDatatype("Behavior::Graph::Interface::C", "float");
    uid = unite(uid, createDatatype("Behavior::Graph::Interface::VHDL", "std_logic_vector(31 downto 0)"));
    isA(uid, Hyperedges{"Behavior::Graph::Interface"});

    // The EDGE algorithm has
    // two inputs: in, weight
    inputIds.clear();
    inputIds=unite(inputIds,instantiateFrom(Hyperedges{"Behavior::Graph::Input"}, "in"));
    // TODO: Currently the weight is in the label. Do we need that input then?
    //inputIds=unite(inputIds,instantiateFrom(Hyperedges{"Behavior::Graph::Input"}, "weight"));
    needsInterface(Hyperedges{"Behavior::Graph::Edge"}, inputIds);
    // one output: out
    providesInterface(Hyperedges{"Behavior::Graph::Edge"}, instantiateFrom(Hyperedges{"Behavior::Graph::Output"}, "out"));

    // MERGE class interfaces (the subclasses inherit them)
    inputIds.clear();
    inputIds=unite(inputIds,instantiateFrom(Hyperedges{"Behavior::Graph::Input"}, "defaultValue"));
    inputIds=unite(inputIds,instantiateFrom(Hyperedges{"Behavior::Graph::Input"}, "bias"));
    providesInterface(Hyperedges{"Behavior::Graph::Merge"}, instantiateFrom(Hyperedges{"Behavior::Graph::Output"}, "merged"));
    // The different subclasses of MERGE
    createAlgorithm("Behavior::Graph::Merge::Sum", "SUM", Hyperedges{"Behavior::Graph::Merge"});
    createAlgorithm("Behavior::Graph::Merge::Product", "PRODUCT", Hyperedges{"Behavior::Graph::Merge"});
    createAlgorithm("Behavior::Graph::Merge::Min", "MIN", Hyperedges{"Behavior::Graph::Merge"});
    createAlgorithm("Behavior::Graph::Merge::Max", "MAX", Hyperedges{"Behavior::Graph::Merge"});
    createAlgorithm("Behavior::Graph::Merge::Mean", "MEAN", Hyperedges{"Behavior::Graph::Merge"});
    createAlgorithm("Behavior::Graph::Merge::Norm", "NORM", Hyperedges{"Behavior::Graph::Merge"});

    // The NODE class subalgorithms
    createAlgorithm("Behavior::Graph::Node::Extern", "EXTERN", Hyperedges{"Behavior::Graph::Node"});
    createAlgorithm("Behavior::Graph::Node::Subgraph", "SUBGRAPH", Hyperedges{"Behavior::Graph::Node"});
    isA(Hyperedges{"Behavior::Graph::Node::Subgraph"}, Hyperedges{Component::Network::NetworkId});

    // Subclassing by arity makes sense
    uid = createAlgorithm("Behavior::Graph::Node::1-1", "arity1", Hyperedges{"Behavior::Graph::Node"});
    // TODO: Make input of INPUT the input of a toplvl node
    // TODO: Make output of OUTPUT the output of a toplvl node
    createAlgorithm("Behavior::Graph::Node::Pipe", "PIPE", uid);
    createAlgorithm("Behavior::Graph::Node::Input", "INPUT", uid);
    createAlgorithm("Behavior::Graph::Node::Output", "OUTPUT", uid);
    createAlgorithm("Behavior::Graph::Node::Divide", "DIVIDE", uid);
    createAlgorithm("Behavior::Graph::Node::Sine", "SIN", uid);
    createAlgorithm("Behavior::Graph::Node::Cosine", "COS", uid);
    createAlgorithm("Behavior::Graph::Node::Tangens", "TAN", uid);
    createAlgorithm("Behavior::Graph::Node::TangensHyperbolicus", "TANH", uid);
    createAlgorithm("Behavior::Graph::Node::ArcusCosine", "ACOS", uid);
    createAlgorithm("Behavior::Graph::Node::ArcusSine", "ASIN", uid);
    createAlgorithm("Behavior::Graph::Node::ArcusTangens", "ATAN", uid);
    createAlgorithm("Behavior::Graph::Node::Logarithm", "LOG", uid);
    createAlgorithm("Behavior::Graph::Node::Exponential", "EXP", uid);
    createAlgorithm("Behavior::Graph::Node::Absolute", "ABS", uid);
    createAlgorithm("Behavior::Graph::Node::SquareRoot", "SQRT", uid);
    // 2-to-1
    uid = createAlgorithm("Behavior::Graph::Node::2-1", "arity2", Hyperedges{"Behavior::Graph::Node"});
    createAlgorithm("Behavior::Graph::Node::ArcusTangens2D", "ATAN2", uid);
    createAlgorithm("Behavior::Graph::Node::Power", "POW", uid);
    createAlgorithm("Behavior::Graph::Node::Modulo", "MOD", uid);
    // 3-to-1
    uid = createAlgorithm("Behavior::Graph::Node::3-1", "arity3", Hyperedges{"Behavior::Graph::Node"});
    createAlgorithm("Behavior::Graph::Node::GreaterZero", ">0", uid);
    createAlgorithm("Behavior::Graph::Node::ApproxZero", "==0", uid);
}

std::string Graph::domainSpecificExport(const UniqueId& uid)
{
    std::map< UniqueId,unsigned > unique2nodeId;
    std::stringstream ss;
    YAML::Node spec;

    // Store toplvl model name
    spec["model"] = get(uid)->label();
    // Handle nodes
    YAML::Node nodesYAML(spec["nodes"]);
    // Important queries
    Hyperedges allParts(componentsOf(Hyperedges{uid}));
    Hyperedges allInputs(inputs());
    Hyperedges allOutputs(outputs());
    Hyperedges superClassesOfAllNodes(algorithmClasses("",Hyperedges{"Behavior::Graph::Node"}));
    Hyperedges nodes(intersect(instancesOf(superClassesOfAllNodes), allParts));
    // TODO: Shall we preserve ids? As we did in hwgraph lib?
    unsigned nodeId = 1;
    for (const UniqueId& nodeUid : nodes)
    {
        YAML::Node nodeYAML;
        Hyperedges superClasses(instancesOf(Hyperedges{nodeUid}, "", TraversalDirection::FORWARD));
        Hyperedges superSuperClasses(subclassesOf(superClasses,"", TraversalDirection::FORWARD));
        if (superClasses.empty())
        {
            std::cout << "No superclass for " << nodeUid << "\n";
            continue;
        }
        // Handle interfaces
        Hyperedges interfaceUids(interfacesOf(Hyperedges{nodeUid}));
        YAML::Node inputsYAML(nodeYAML["inputs"]);
        for (const UniqueId& inputUid : intersect(interfaceUids, allInputs))
        {
            YAML::Node inputYAML;
            inputYAML["name"] = get(inputUid)->label();
            Hyperedges mergeUid(childrenOf(endpointsOf(Hyperedges{inputUid}),"",TraversalDirection::INVERSE));
            inputYAML["type"] = get(*mergeUid.begin())->label();
            //inputYAML["bias"] = get(*superClasses2.begin())->label();
            //inputYAML["default"] = get(*superClasses2.begin())->label();
            inputsYAML.push_back(inputYAML);
        }
        YAML::Node outputsYAML(nodeYAML["outputs"]);
        for (const UniqueId& outputUid : intersect(interfaceUids, allOutputs))
        {
            YAML::Node outputYAML;
            outputYAML["name"] = get(outputUid)->label();
            outputsYAML.push_back(outputYAML);
        }
        nodeYAML["id"] = nodeId;
        nodeYAML["name"] = get(nodeUid)->label();
        // Special cases for SUBGRAPH && EXTERN
        if (superSuperClasses.count("Behavior::Graph::Node::Subgraph"))
        {
            nodeYAML["type"] = get("Behavior::Graph::Node::Subgraph")->label();
            nodeYAML["subgraph_name"] = get(*superClasses.begin())->label();
        }
        else if (superSuperClasses.count("Behavior::Graph::Node::Extern"))
        {
            nodeYAML["type"] = get("Behavior::Graph::Node::Extern")->label();
            nodeYAML["extern_name"] = get(*superClasses.begin())->label();
        } else {
            nodeYAML["type"] = get(*superClasses.begin())->label();
        }
        nodesYAML.push_back(nodeYAML);
        unique2nodeId[nodeUid] = nodeId++;
    }

    // Handle edges
    YAML::Node edgesYAML(spec["edges"]);
    Hyperedges edges(intersect(instancesOf(algorithmClasses("",Hyperedges{"Behavior::Graph::Edge"})), allParts));
    for (const UniqueId& edgeUid : edges)
    {
        YAML::Node edgeYAML;
        edgeYAML["weight"] = std::stof(get(edgeUid)->label());
        // Get edge input and follow chain
        Hyperedges fromOutputUid(endpointsOf(interfacesOf(Hyperedges{edgeUid},"in")));
        // TODO: If label can be converted to a unsigned int ... use fromNodeOutputIdx
        edgeYAML["fromNodeOutput"] = get(*fromOutputUid.begin())->label();
        Hyperedges fromNodeUid(childrenOf(fromOutputUid,"",TraversalDirection::INVERSE));
        edgeYAML["fromNode"] = get(*fromNodeUid.begin())->label();
        // Get edge output and follow chain
        // Here we will get to a merge in between!!!
        Hyperedges toMergeInputUid(endpointsOf(interfacesOf(Hyperedges{edgeUid},"out"),"",TraversalDirection::INVERSE));
        Hyperedges toMergeNodeUid(childrenOf(toMergeInputUid,"",TraversalDirection::INVERSE));
        Hyperedges toMergeOutputUid(interfacesOf(toMergeNodeUid, "merged"));
        Hyperedges toInputUid(endpointsOf(toMergeOutputUid,"",TraversalDirection::INVERSE));
        Hyperedges toNodeUid(childrenOf(toInputUid,"",TraversalDirection::INVERSE));
        edgeYAML["toNodeInput"] = get(*toInputUid.begin())->label();
        edgeYAML["toNode"] = get(*toNodeUid.begin())->label();
        edgesYAML.push_back(edgeYAML);
    }

    ss << spec;
    return ss.str();
}

Hyperedges Graph::getMergesOfInput(const Hyperedges& inputs, const std::string& label)
{
    Hyperedges allMerges = instancesOf(algorithmClasses("", Hyperedges{"Behavior::Graph::Merge"}), label);
    Hyperedges allEndpoints = endpointsOf(inputs, "merged"); // all outputs of some merges?
    Hyperedges allParentsOfEndpoints = childrenOf(allEndpoints, label, Software::Graph::TraversalDirection::INVERSE);
    return intersect(allMerges, allParentsOfEndpoints);
}

bool Graph::domainSpecificImport(const std::string& serialized)
{
    // Map from old ids (in file) to new ids
    std::map<UniqueId, Hyperedges> old2new;
    YAML::Node spec = YAML::Load(serialized);
    // Create a network class
    std::string name("NONAME");
    if (spec["model"].IsDefined())
        name = spec["model"].as<std::string>();
    Hyperedges partsOfNet;
    Hyperedges networkUid(createNetwork("Behavior::Graph::Node::Subgraph::"+name, name, Hyperedges{"Behavior::Graph::Node::Subgraph"}));
    if (!spec["nodes"].IsDefined())
        return false;
    const YAML::Node& nodesYAML(spec["nodes"]);
    if (nodesYAML.IsDefined())
    {
        for (YAML::Node::const_iterator nit = nodesYAML.begin(); nit != nodesYAML.end(); ++nit)
        {
            const YAML::Node& nodeYAML(*nit);
            const UniqueId& id(nodeYAML["id"].as<UniqueId>());
            const std::string& type(nodeYAML["type"].as<std::string>());
            // TODO: We could use the domain concept here :)
            Hyperedges super(algorithmClasses(type));
            if (super.empty())
            {
                std::cout << "Warning, superclass " << type << " not found\n";
                continue;
            }
            std::string label(id);
            if (nodeYAML["name"].IsDefined())
                label = nodeYAML["name"].as<std::string>();
            const YAML::Node& inputsYAML(nodeYAML["inputs"]);
            const YAML::Node& outputsYAML(nodeYAML["outputs"]);
            const YAML::Node& subgraph_nameYAML(nodeYAML["subgraph_name"]);
            // TODO: What about extern_name ?

            // For SUBGRAPH and EXTERN nodes we first have to create a subclass & interfaces (if it does not exist yet)
            if ((type == "SUBGRAPH") || (type == "EXTERN"))
            {
                // Define the superclassLabel
                std::string superLabel(label);
                if (subgraph_nameYAML.IsDefined())
                    superLabel = subgraph_nameYAML.as<std::string>();
                // super contains now either SUBGRAPH || EXTERN
                Hyperedges sub(algorithmClasses(superLabel, super));
                if (sub.empty())
                {
                    // First occurrence: Create a subclass with interfaces
                    std::cout << "Create subclass " << superLabel << " of type " << type << "\n";
                    sub = createAlgorithm(superLabel, superLabel, super);
                }
                // The new sub class is the super class of the following code
                super = sub;
            }

            // Instantiate node
            std::cout << "Instantiating " << label << " of type " << super << "\n";
            Hyperedges uid(instantiateComponent(super, label));
            old2new[id] = uid;
            partsOfNet = unite(partsOfNet, uid);

            // For every input, create and connect the correct MERGE algorithm
            Hyperedges inputIds;
            if (inputsYAML.IsDefined())
            {
                for (YAML::Node::const_iterator it = inputsYAML.begin(); it != inputsYAML.end(); it++)
                {
                    // Create input
                    const YAML::Node& inputYAML(*it);
                    // Search by string or idx
                    std::string inputLabel;
                    if (inputYAML["name"].IsDefined())
                    {
                        inputLabel = inputYAML["name"].as<std::string>();
                    } else {
                        inputLabel = inputYAML["idx"].as<std::string>();
                    }
                    Hyperedges inputOfComponent(instantiateFrom(Hyperedges{"Behavior::Graph::Input"},inputLabel));
                    inputIds = unite(inputIds, inputOfComponent);
                    std::cout << "Create input " << inputLabel << " for " << label << "\n";

                    // Create merge
                    const std::string& mergeType(inputYAML["type"].as<std::string>());
                    const std::string& mergeBias(inputYAML["bias"].as<std::string>());
                    const std::string& mergeDefault(inputYAML["default"].as<std::string>());
                    // Search for merge class
                    Hyperedges mergeClasses(algorithmClasses(mergeType, Hyperedges{"Behavior::Graph::Merge"}));
                    if (!mergeClasses.size())
                    {
                        std::cout << "Warning, could not find merge type " << mergeType << "\n";
                        continue;
                    }
                    // Instantiate merge
                    std::cout << "Instantiating merge " << mergeType << " for input " << inputLabel << "\n";
                    Hyperedges mergeUid(instantiateComponent(mergeClasses));
                    partsOfNet = unite(partsOfNet, mergeUid);
                    Hyperedges outputOfMerge(intersect(interfacesOf(mergeUid, "merged"), outputs("merged")));
                    std::cout << dependsOn(inputOfComponent, outputOfMerge) << "\n";
                }
                needsInterface(uid, inputIds);
            }

            // Create specified outputs
            Hyperedges outputIds;
            if (outputsYAML.IsDefined())
            {
                for (YAML::Node::const_iterator it = outputsYAML.begin(); it != outputsYAML.end(); it++)
                {
                    const YAML::Node& outputYAML(*it);
                    std::string outputLabel;
                    if (outputYAML["name"].IsDefined())
                    {
                        outputLabel = outputYAML["name"].as<std::string>();
                    } else {
                        outputLabel = outputYAML["idx"].as<std::string>();
                    }
                    outputIds = unite(outputIds, instantiateFrom(Hyperedges{"Behavior::Graph::Output"},outputLabel));
                    std::cout << "Create output " << outputLabel << " for " << label << "\n";
                }
                providesInterface(uid, outputIds);
            }

            // TODO: Is it that simple?
            // For INPUT nodes: Make their input(s) the input(s) of the toplvl node
            if (type == "INPUT")
            {
                std::cout << "Export inputs " << inputIds << std::endl;
                needsInterface(networkUid, inputIds);
            }
            // For OUTPUT nodes: Make their output(s) the output(s) of the toplvl node
            if (type == "OUTPUT")
            {
                std::cout << "Export outputs " << outputIds << std::endl;
                providesInterface(networkUid, outputIds);
            }
        }
    } else {
        std::cout << "No nodes section\n";
        return false;
    }

    // Treat edges
    const YAML::Node& edgesYAML(spec["edges"]);
    if (edgesYAML.IsDefined())
    {
        for (YAML::Node::const_iterator eit = edgesYAML.begin(); eit != edgesYAML.end(); ++eit)
        {
            const YAML::Node& edgeYAML(*eit);
            const std::string& label(edgeYAML["weight"].as<std::string>());
            std::string fromLabel, fromOutputLabel, toLabel, toInputLabel;
            // We can also connect by names, right? Yes :)
            Hyperedges fromNodeIds;
            if (edgeYAML["fromNode"].IsDefined())
            {
                // Find UnqiueIds by label
                fromLabel = edgeYAML["fromNode"].as<std::string>();
                fromNodeIds = algorithms(fromLabel);
            } else {
                // Find UniqueIds by remapped ids
                fromLabel = edgeYAML["fromNodeId"].as<std::string>();
                fromNodeIds = old2new[fromLabel];
            }
            if (fromNodeIds.empty())
            {
                std::cout << "Could not find node " << fromLabel << "\n";
                continue;
            }
            Hyperedges toNodeIds;
            if (edgeYAML["toNode"].IsDefined())
            {
                // Find UnqiueIds by label
                toLabel = edgeYAML["toNode"].as<std::string>();
                toNodeIds = algorithms(toLabel);
            } else {
                // Find UniqueIds by remapped ids
                toLabel = edgeYAML["toNodeId"].as<std::string>();
                toNodeIds = old2new[toLabel];
            }
            if (toNodeIds.empty())
            {
                std::cout << "Could not find node " << toLabel << "\n";
                continue;
            }
            if (edgeYAML["fromNodeOutput"].IsDefined())
            {
                // Find UnqiueIds by label
                fromOutputLabel = edgeYAML["fromNodeOutput"].as<std::string>();
            } else {
                // Find UniqueIds by remapped ids
                fromOutputLabel = edgeYAML["fromNodeOutputIdx"].as<std::string>();
            }
            Hyperedges fromNodeOutputIds(intersect(interfacesOf(fromNodeIds, fromOutputLabel), outputs(fromOutputLabel)));
            if (fromNodeOutputIds.empty())
            {
                std::cout << "Node " << fromLabel << " has no output " << fromOutputLabel << "\n";
                continue;
            }
            if (edgeYAML["toNodeInput"].IsDefined())
            {
                // Find UnqiueIds by label
                toInputLabel = edgeYAML["toNodeInput"].as<std::string>();
            } else {
                // Find UniqueIds by remapped ids
                toInputLabel = edgeYAML["toNodeInputIdx"].as<std::string>();
            }
            Hyperedges toNodeInputIds(intersect(interfacesOf(toNodeIds, toInputLabel), inputs(toInputLabel)));
            if (toNodeInputIds.empty())
            {
                std::cout << "Node " << toLabel << " has no input " << toInputLabel << "\n";
                continue;
            }

            //  OPTIMIZATION: Create a new input to the merge function on demand
            Hyperedges mergeIds(getMergesOfInput(toNodeInputIds));
            Hyperedges unconnectedInputs;
            for (const UniqueId& mergeId : mergeIds)
            {
                Hyperedges newInputOfMerge(instantiateFrom(Hyperedges{"Behavior::Graph::Input"}));
                needsInterface(Hyperedges{mergeId}, newInputOfMerge);
                unconnectedInputs = unite(unconnectedInputs, newInputOfMerge);
            }

            // Now we can instantiate and connect
            std::cout << "Connecting " << fromLabel << "," << fromOutputLabel << " to " << toLabel << "," << toInputLabel << "\n";
            Hyperedges uid(instantiateComponent(Hyperedges{"Behavior::Graph::Edge"}, label));
            partsOfNet = unite(partsOfNet, uid);
            std::cout << dependsOn(interfacesOf(uid,"in"), fromNodeOutputIds) << " ";
            std::cout << dependsOn(unconnectedInputs, interfacesOf(uid,"out")) << "\n";
        }
    }
    // Make all stuff part of the network
    partOfNetwork(partsOfNet, networkUid);
    return true;
}

}