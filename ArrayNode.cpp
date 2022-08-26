#include "ArrayNode.hpp"

ArrayNode::ArrayNode(int size) {
	array = new std::atomic<Node*>[size];
	for(int i = 0; i < size; i++) array[i] = nullptr;
}

ArrayNode::~ArrayNode() {}
