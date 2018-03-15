#include "merkletree.h"

using namespace std;

int main(){

	// initialize leaves
	vector<char*> leaves(4);
	leaves[0] = "92cdf578c47085a5992256f0dcf97d0b"; // 1
	leaves[1] = "31237cdb79ae1dfa7ffb87cde7ea8a80"; // 2
	leaves[2] = "581348337b0f3e148620173daaa5f94d"; // 3
	leaves[3] = "eb8649214997574e20c464388a172420"; // 4
    // [[[1]+[2]] + [[3]+[4]]]
    // [6eb00c878931c099ca1a48951db76505+812f428b2d97039f950d63f1d55e9f4d]
    // 42c8ed715efc42b251e38ea429322852 // root

	// initialize merkletree
	merkletree mtree = merkletree(leaves);

	char* root = mtree.root();
	// get proof
	char* sample_leaf = leaves[0];
	vector<ProofNode> proof = mtree.proof(sample_leaf);
  	printf("root: %s\n",mtree.root());

	// verify proof
	bool verified = verifyProof(sample_leaf,root,proof);
  	printf("ver: %d\n",verified);

	// Push leaf
	// This will change value of some nodes
	char* newleaf = "70b201352f24bf1c9770b99f8f712018"; // 5
	mtree.pushleaf(newleaf);

	char* newroot = mtree.root();
	printf("new root : %s\n",mtree.root());

	// verify new leaf
	vector<ProofNode> newproof = mtree.proof(newleaf);
	bool newverified = verifyProof(newleaf,newroot,newproof);

	printf("new ver : %d\n",newverified);

	// serialize and deserialize
	char *ser_str = serialize(proof);
	printf("result of serialization : %s\n",ser_str);
	vector<ProofNode> sdproof = deserialize(ser_str);

	// verify deserialized proof
	bool dsverified = verifyProof(sample_leaf,root,sdproof);
	printf("verify d&s proof : %d\n",dsverified);
}
