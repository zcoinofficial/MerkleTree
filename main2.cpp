#include "merkletree.h"
#include <time.h>

#define TEST_INSERT 0
// #define DUMP_TREE

using namespace std;

int main()
{

    int Leaf;
    scanf("%d",&Leaf);

    vector<char*> v(Leaf-TEST_INSERT);

    for(int i =0; i<Leaf-TEST_INSERT; i++)
    {
        char *buff = new char[33];
        scanf("%33s\n",buff );
        v[i] = buff;
    }

    clock_t t;
    t = clock();
    merkletree mtree = merkletree(v);



    for(int i=Leaf-TEST_INSERT; i<Leaf; i++)
    {
        char* buff = new char[33];
        scanf("%33s\n",buff);
        v.push_back(buff);
        mtree.pushleaf(v[i]);

    }

#ifdef DUMP_TREE
    for(int i=0; i<v.size(); i++)
        printf("leave : %s\n",v[i]);
    for(int i=0; i<mtree.tree.size(); i++)
        printf("node : %s\n",mtree.tree[i]);
#endif





    printf("root : %s\n",mtree.root());
    t = clock() - t;
    // printf("[build mtree] took %d clocks (%f secs)\n",t,(float)t/CLOCKS_PER_SEC);

    t = clock();


    for(int i=0; i<v.size(); i++)
    {
        char* leaf = v[i];
    //	printf("leaf : %s\nroot : %s\n",leaf,mtree.root());
        vector<ProofNode> proof = mtree.proof(leaf);

    //	for(int i = 0 ;i<proof.size();i++){
    //		printf("parent : %s\nleft : %s\nright : %s\n",proof[i].parent,proof[i].right,proof[i].left);
    //	}

        t = clock() - t;
        // printf("[get proof] took %d clocks (%f secs)\n",t,(float)t/CLOCKS_PER_SEC);

        t = clock();

        bool verproof = verifyProof(leaf,mtree.root(),proof);

        t = clock() - t;
        // printf("[verify proof][ => %d ] took %d clocks (%f secs)\n",verproof,t,(float)t/CLOCKS_PER_SEC);
    }


    for(int i=0; i<mtree.size(); i++)
    {
        char* leaf = mtree.tree[i];
        vector<ProofNode> proof = mtree.proof(leaf);

        //	for(int i = 0 ;i<proof.size();i++){
        //		printf("parent : %s\nleft : %s\nright : %s\n",proof[i].parent,proof[i].right,proof[i].left);
        //
        // }

    //	char *sr = serialize(old_proof);
    //	vector<ProofNode> proof = deserialize(sr);

        t = clock() - t;
        //printf("[get proof] took %d clocks (%f secs)\n",t,(float)t/CLOCKS_PER_SEC);

        t = clock();

        //          bool verproof = verifyProof(leaf,mtree.root(),proof);

        bool verproof = verifyProof(leaf,mtree.root(),proof);

        t = clock() - t;
        if(verproof)
        {
            printf("[verify proof][ =< %d ] took %d clocks (%f secs)\n",verproof,t,(float)t/CLOCKS_PER_SEC);
        }
    }


}
