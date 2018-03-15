#include "binarytree.h"
#include <string.h>
#include <iostream>

#include "argon2ref/blake2.h"
#include "argon2ref/blake2-impl.h"

#define BLAKE2_LENGTH 32
#define BLAKE2_DIGEST_LENGTH 16
#define NODE_LENGTH 33

void calBlake2(char* inp,char out_buff[BLAKE2_LENGTH])
{
    unsigned char hash[BLAKE2_DIGEST_LENGTH];

    ablake2b_state s;
    ablake2b_init(&s,BLAKE2_LENGTH);
    ablake2b_update(&s,inp,strlen(inp));
    ablake2b_final(&s,hash,BLAKE2_LENGTH);

    for(int i = 0; i < 16; i++)
    {
        sprintf(out_buff + (i * 2), "%02x", hash[i]);
    }
}

using namespace std;

struct ProofNode
{
    char* hash;
    bool isRight;
    ProofNode()
    {
        isRight= false;
        hash = new char[BLAKE2_LENGTH];
    }
};

// buff
char* serialize(vector<ProofNode>& proof) // Writes the given OBJECT data to the given file name.
{
    char *buff = new char[proof.size()*NODE_LENGTH+1];
    buff[proof.size()*NODE_LENGTH]=0;
    for(int i =0; i<proof.size(); i++)
    {
        memcpy(buff+NODE_LENGTH*i+1,proof[i].hash,BLAKE2_LENGTH);
        buff[NODE_LENGTH*i] = proof[i].isRight? '1':'0';
    }
    return buff;
};

vector<ProofNode> deserialize(const char* strdata) // Reads the given file and assigns the data to the given OBJECT.
{
    size_t datalen = strlen(strdata);
    vector<ProofNode> proof(datalen/NODE_LENGTH);

    for(int i = 0 ; i<proof.size(); i++)
    {
        memcpy(proof[i].hash,strdata+NODE_LENGTH*i+1,BLAKE2_LENGTH);
        proof[i].isRight = strdata[NODE_LENGTH*i]!='0';
    }

    return proof;
};

// combin and hash by sha256
static void combin(char* leftData,char* rightData,char out_buff[BLAKE2_LENGTH+1])
{
    unsigned char hash[BLAKE2_DIGEST_LENGTH]; // 128/8
    ablake2b_state s;
    ablake2b_init(&s,BLAKE2_LENGTH);
    ablake2b_update(&s,leftData,strlen(leftData));
    ablake2b_update(&s,rightData,strlen(rightData));
    ablake2b_final(&s,hash,BLAKE2_LENGTH);

    for(int i = 0; i < BLAKE2_DIGEST_LENGTH; i++)
    {
        sprintf(out_buff + (i * 2), "%02x", hash[i]);
    }
}

bool verifyProof(char* verifyLeaf,char* expectedMerkleRoot,vector<ProofNode> proof)
{

    size_t vLeafLen = strlen(verifyLeaf);
    char* leaf = (char*)malloc(vLeafLen+1);
    memcpy(leaf,verifyLeaf,vLeafLen);
    if(proof.size() == 0 )
    {
        if( strcmp(leaf,expectedMerkleRoot)== 0)
            return true;
        return false;
    }

    // the merkle root should be the parent of the last part
    char* actualMekleRoot = proof[proof.size() -1].hash;

    if( strcmp(actualMekleRoot,expectedMerkleRoot)!=0 )
        return false;

    for(int pIdx =0; pIdx<proof.size()-1; pIdx++)
    {

        if (proof[pIdx].isRight)
        {
            combin(leaf,proof[pIdx].hash,leaf);
        }
        else
        {
            combin(proof[pIdx].hash,leaf,leaf);
        }


    }

    bool valid = strcmp(leaf,expectedMerkleRoot) == 0;
    free(leaf);
    return valid;
}

class merkletree
{
public:
    vector<char*> tree;

    merkletree() {}
    merkletree(vector<char*> leaves)
    {
        tree = computeTree(combin,leaves);
    }

    size_t size()
    {
        return tree.size();
    }
    char* root()
    {
        return tree[0];
    }

    vector<char*> computeTree(void (*combineFn)(char*,char*,char*),vector<char*> leaves)
    {
        // compute nodeCount and create vector<T> tree
        int nodeCount = leafCountToNodeCount(leaves.size());
        int delta = nodeCount - leaves.size();
        vector<char*> tree(nodeCount);

        // deep copy
        for(int i = 0 ; i<leaves.size(); i++)
        {
            tree[delta + i] = new char[BLAKE2_LENGTH];
            memcpy(tree[delta + i],leaves[i],BLAKE2_LENGTH);
        }
        int idx = nodeCount-1;
        while(idx > 0)
        {
            int parent = (idx -1)/2;

            tree[parent] = new char[BLAKE2_LENGTH];
            combineFn(tree[idx-1],tree[idx],tree[parent]);

            idx-=2;
        }

        return tree;
    }



    vector<ProofNode> proof(char* leafData)
    {
        int idx = findLeaf(tree,leafData);
        if(idx == -1)
            return vector<ProofNode>();
        int proofArrSize = floor( log(tree.size())/ log(2) )+1;
        vector<ProofNode> proof(proofArrSize);

        // record seld
        int proofIdx = 0;

        while(idx > 0 )
        {
            idx = getBro(tree,idx);
            proof[proofIdx].isRight = idx%2==0;
            proof[proofIdx++].hash = tree[idx];
            idx = getParent(tree,idx);
        }

        // push root
        proof[proofIdx].isRight = false;
        proof[proofIdx++].hash = tree[idx];


        proof.resize(proofIdx);

        return proof;
    }


    void pushleaf(char* leaf)
    {
        pushleafworker(combin,leaf);
    }


    void pushleafworker(void (*combineFn)(char*,char*,char*),char* leaf)
    {

        // push two
        tree.push_back(new char[BLAKE2_LENGTH]);
        tree.push_back(new char[BLAKE2_LENGTH]);

        int pidx = getParent(tree,tree.size()-1);

        // push parent and newleaf
        memcpy(tree[tree.size()-2],tree[pidx],BLAKE2_LENGTH);
        memcpy(tree[tree.size()-1],leaf,BLAKE2_LENGTH);

        // climb up and compute
        int idx = tree.size()-1;
        while(idx > 0)
        {
            idx = getParent(tree,idx);
            char *buff = new char[BLAKE2_LENGTH+1];
            combineFn(tree[getLeft(tree,idx)],tree[getRight(tree,idx)],buff);
            tree[idx] = buff;
        }
    }


};
