/*
 * Argon2 reference source code package - reference C implementations
 *
 * Copyright 2015
 * Daniel Dinu, Dmitry Khovratovich, Jean-Philippe Aumasson, and Samuel Neves
 *
 * You may use this work under the terms of a Creative Commons CC0 1.0 
 * License/Waiver or the Apache Public License 2.0, at your option. The terms of
 * these licenses can be found at:
 *
 * - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
 * - Apache 2.0        : http://www.apache.org/licenses/LICENSE-2.0
 *
 * You should have received a copy of both of these licenses along with this
 * software. If not, they may be obtained at the above URLs.
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "argon2.h"
#include "core.h"

#include "blamka-round-ref.h"
#include "blake2-impl.h"
#include "blake2.h"


/*
 * Function fills a new memory block and optionally XORs the old block over the new one.
 * @next_block must be initialized.
 * @param prev_block Pointer to the previous block
 * @param ref_block Pointer to the reference block
 * @param next_block Pointer to the block to be constructed
 * @param with_xor Whether to XOR into the new block (1) or just overwrite (0)
 * @pre all block pointers must be valid
 */
static void fill_block(const block *prev_block, const block *ref_block,
                       block *next_block, int with_xor, uint32_t block_header[4]) {
    block blockR, block_tmp;
    unsigned i;

    copy_block(&blockR, ref_block);

    xor_block(&blockR, prev_block);

    copy_block(&block_tmp, &blockR);
    /* Now blockR = ref_block + prev_block and block_tmp = ref_block + prev_block */
    if (with_xor) {
        /* Saving the next block contents for XOR over: */
        xor_block(&block_tmp, next_block);
        /* Now blockR = ref_block + prev_block and
           block_tmp = ref_block + prev_block + next_block */
    }
//	blockR.v[16] = ((uint64_t*)block_header)[0];
//	blockR.v[17] = ((uint64_t*)block_header)[1];
	memcpy(&blockR.v[16], (uint64_t*)block_header, 2 * sizeof(uint64_t));
//printf("block header in cpu %llx %llx %llx %llx\n", blockR.v[15], blockR.v[16], blockR.v[17], blockR.v[18]);

    /* Apply Blake2 on columns of 64-bit words: (0,1,...,15) , then
       (16,17,..31)... finally (112,113,...127) */
    for (i = 0; i < 8; ++i) {
        BLAKE2_ROUND_NOMSG(
            blockR.v[16 * i], blockR.v[16 * i + 1], blockR.v[16 * i + 2],
            blockR.v[16 * i + 3], blockR.v[16 * i + 4], blockR.v[16 * i + 5],
            blockR.v[16 * i + 6], blockR.v[16 * i + 7], blockR.v[16 * i + 8],
            blockR.v[16 * i + 9], blockR.v[16 * i + 10], blockR.v[16 * i + 11],
            blockR.v[16 * i + 12], blockR.v[16 * i + 13], blockR.v[16 * i + 14],
            blockR.v[16 * i + 15]);
    }

    /* Apply Blake2 on rows of 64-bit words: (0,1,16,17,...112,113), then
       (2,3,18,19,...,114,115).. finally (14,15,30,31,...,126,127) */
    for (i = 0; i < 8; i++) {
        BLAKE2_ROUND_NOMSG(
            blockR.v[2 * i], blockR.v[2 * i + 1], blockR.v[2 * i + 16],
            blockR.v[2 * i + 17], blockR.v[2 * i + 32], blockR.v[2 * i + 33],
            blockR.v[2 * i + 48], blockR.v[2 * i + 49], blockR.v[2 * i + 64],
            blockR.v[2 * i + 65], blockR.v[2 * i + 80], blockR.v[2 * i + 81],
            blockR.v[2 * i + 96], blockR.v[2 * i + 97], blockR.v[2 * i + 112],
            blockR.v[2 * i + 113]);
    }

    copy_block(next_block, &block_tmp);
    xor_block(next_block, &blockR);
}

static void fill_block_withIndex(const block *prev_block, const block *ref_block,
	block *next_block, int with_xor, uint32_t block_header[8], uint64_t index) {
	block blockR, block_tmp;
	uint64_t TheIndex[2] = {0,index};
	unsigned i;

	copy_block(&blockR, ref_block);

	xor_block(&blockR, prev_block);

	copy_block(&block_tmp, &blockR);
	/* Now blockR = ref_block + prev_block and block_tmp = ref_block + prev_block */
	if (with_xor) {
		/* Saving the next block contents for XOR over: */
		xor_block(&block_tmp, next_block);
		/* Now blockR = ref_block + prev_block and
		block_tmp = ref_block + prev_block + next_block */
	}
	//	blockR.v[16] = ((uint64_t*)block_header)[0];
	//	blockR.v[17] = ((uint64_t*)block_header)[1];
	memcpy(&blockR.v[14], TheIndex, 2 * sizeof(uint64_t)); //index here
	memcpy(&blockR.v[16], (uint64_t*)block_header, 2 * sizeof(uint64_t));
	memcpy(&blockR.v[18], (uint64_t*)(block_header + 4), 2 * sizeof(uint64_t));
	//printf("block header in cpu %llx %llx %llx %llx\n", blockR.v[15], blockR.v[16], blockR.v[17], blockR.v[18]);

	/* Apply Blake2 on columns of 64-bit words: (0,1,...,15) , then
	(16,17,..31)... finally (112,113,...127) */
	for (i = 0; i < 8; ++i) {
		BLAKE2_ROUND_NOMSG(
			blockR.v[16 * i], blockR.v[16 * i + 1], blockR.v[16 * i + 2],
			blockR.v[16 * i + 3], blockR.v[16 * i + 4], blockR.v[16 * i + 5],
			blockR.v[16 * i + 6], blockR.v[16 * i + 7], blockR.v[16 * i + 8],
			blockR.v[16 * i + 9], blockR.v[16 * i + 10], blockR.v[16 * i + 11],
			blockR.v[16 * i + 12], blockR.v[16 * i + 13], blockR.v[16 * i + 14],
			blockR.v[16 * i + 15]);
	}

	/* Apply Blake2 on rows of 64-bit words: (0,1,16,17,...112,113), then
	(2,3,18,19,...,114,115).. finally (14,15,30,31,...,126,127) */
	for (i = 0; i < 8; i++) {
		BLAKE2_ROUND_NOMSG(
			blockR.v[2 * i], blockR.v[2 * i + 1], blockR.v[2 * i + 16],
			blockR.v[2 * i + 17], blockR.v[2 * i + 32], blockR.v[2 * i + 33],
			blockR.v[2 * i + 48], blockR.v[2 * i + 49], blockR.v[2 * i + 64],
			blockR.v[2 * i + 65], blockR.v[2 * i + 80], blockR.v[2 * i + 81],
			blockR.v[2 * i + 96], blockR.v[2 * i + 97], blockR.v[2 * i + 112],
			blockR.v[2 * i + 113]);
	}

	copy_block(next_block, &block_tmp);
	xor_block(next_block, &blockR);
}


static void next_addresses(block *address_block, block *input_block,
                           const block *zero_block, uint32_t block_header[4]) {
    input_block->v[6]++;
    fill_block(zero_block, input_block, address_block, 0, block_header);
    fill_block(zero_block, address_block, address_block, 0, block_header);
}

void fill_segment(const argon2_instance_t *instance,
                  argon2_position_t position) {
    block *ref_block = NULL, *curr_block = NULL;
	uint32_t *zPrevBlock = NULL, *zRefBlock = NULL;
    block address_block, input_block, zero_block;
    uint64_t pseudo_rand, ref_index, ref_lane;
    uint32_t prev_offset, curr_offset;
    uint32_t starting_index;
    uint32_t i;
    int data_independent_addressing;

    if (instance == NULL) {
        return;
    }

    data_independent_addressing =
        (instance->type == Argon2_i) ||
        (instance->type == Argon2_id && (position.pass == 0) &&
         (position.slice < ARGON2_SYNC_POINTS / 2));

    if (data_independent_addressing) {
        init_block_value(&zero_block, 0);
        init_block_value(&input_block, 0);
 
        input_block.v[0] = position.pass;
        input_block.v[1] = position.lane;
        input_block.v[2] = position.slice;
        input_block.v[3] = instance->memory_blocks;
        input_block.v[4] = instance->passes;
        input_block.v[5] = instance->type;
    }

    starting_index = 0;

    if ((0 == position.pass) && (0 == position.slice)) {
        starting_index = 2; /* we have already generated the first two blocks */

        /* Don't forget to generate the first block of addresses: */
        if (data_independent_addressing) {
            next_addresses(&address_block, &input_block, &zero_block, instance->block_header);
        }
    }
    /* Offset of the current block */
    curr_offset = position.lane * instance->lane_length +
                  position.slice * instance->segment_length + starting_index;


    if (0 == curr_offset % instance->lane_length) {
        /* Last block in this lane */
        prev_offset = curr_offset + instance->lane_length - 1;
    } else {
        /* Previous block */
        prev_offset = curr_offset - 1;
    }
int truc = 0;
    for (i = starting_index; i < instance->segment_length;
         ++i, ++curr_offset, ++prev_offset) {
truc++;
        /*1.1 Rotating prev_offset if needed */
        if (curr_offset % instance->lane_length == 1) {
            prev_offset = curr_offset - 1;
        }

        /* 1.2 Computing the index of the reference block */
        /* 1.2.1 Taking pseudo-random value from the previous block */
        if (data_independent_addressing) {
            if (i % ARGON2_ADDRESSES_IN_BLOCK == 0) {
                next_addresses(&address_block, &input_block, &zero_block, instance->block_header);
            }
            pseudo_rand = address_block.v[i % ARGON2_ADDRESSES_IN_BLOCK];
        } else {
            pseudo_rand = instance->memory[prev_offset].v[0];
        }
//if (i==starting_index)
//printf("\n");
//printf(" *** prevOffset %d ***",prev_offset);
//	instance->lane_length, instance->segment_length,i,pseudo_rand);
        /* 1.2.2 Computing the lane of the reference block */
        ref_lane = ((pseudo_rand >> 32)) % instance->lanes;

        if ((position.pass == 0) && (position.slice == 0)) {
            /* Can not reference other lanes yet */
            ref_lane = position.lane;
        }

        /* 1.2.3 Computing the number of possible reference block within the
         * lane.
         */
        position.index = i;




        ref_index = index_alpha(instance, &position, pseudo_rand & 0xFFFFFFFF,
                                ref_lane == position.lane);

        /* 2 Creating a new block */
        ref_block =
            instance->memory + instance->lane_length * ref_lane + ref_index;

        curr_block = instance->memory + curr_offset;
		uint64_t TheBlockIndex = instance->lane_length * ref_lane + ref_index;
/*
		zRefBlock  = &instance->TheRefBlock + curr_offset;
		zPrevBlock = &instance->ThePrevBlock + curr_offset;
*/
        if (ARGON2_VERSION_10 == instance->version) {
            /* version 1.2.1 and earlier: overwrite, not XOR */

//            fill_block(instance->memory + prev_offset, ref_block, curr_block, 0, instance->block_header);
	fill_block_withIndex(instance->memory + prev_offset, ref_block, curr_block, 0, instance->block_header, TheBlockIndex);
        } else {
            if(0 == position.pass) {
//                        fill_block(instance->memory + prev_offset, ref_block,curr_block, 0, instance->block_header);
				fill_block_withIndex(instance->memory + prev_offset, ref_block, curr_block, 0, instance->block_header, TheBlockIndex);
				curr_block->ref_block = instance->lane_length * ref_lane + ref_index;
				curr_block->prev_block = prev_offset | (instance->lane_length * ref_lane + ref_index) << 32;;
//				uint64_t zHistory =  (prev_offset) | (instance->lane_length * ref_lane + ref_index) << 32;
//				curr_block->BlokHistory = zHistory;
            } else {


//                fill_block(instance->memory + prev_offset, ref_block,curr_block, 1, instance->block_header);
				fill_block_withIndex(instance->memory + prev_offset, ref_block, curr_block, 1, instance->block_header, TheBlockIndex);
            }
        }
    }
}

void fill_segment_noinde(const argon2_instance_t *instance,
	argon2_position_t position) {
	block *ref_block = NULL, *curr_block = NULL;
	block address_block, input_block, zero_block;
	uint64_t pseudo_rand, ref_index, ref_lane;
	uint32_t prev_offset, curr_offset;
	uint32_t starting_index;
	uint32_t i;
	int data_independent_addressing;

	if (instance == NULL) {
		return;
	}


	starting_index = 0;

	if ((0 == position.pass) && (0 == position.slice)) {
		starting_index = 2; /* we have already generated the first two blocks */
	}

	/* Offset of the current block */
	curr_offset = position.lane * instance->lane_length +
		position.slice * instance->segment_length + starting_index;

	if (0 == curr_offset % instance->lane_length) {
		/* Last block in this lane */
		prev_offset = curr_offset + instance->lane_length - 1;
	}
	else {
		/* Previous block */
		prev_offset = curr_offset - 1;
	}

	for (i = starting_index; i < instance->segment_length;
		++i, ++curr_offset, ++prev_offset) {


		/*1.1 Rotating prev_offset if needed */
		if (curr_offset % instance->lane_length == 1) {
			prev_offset = curr_offset - 1;
		}

		/* 1.2 Computing the index of the reference block */
		/* 1.2.1 Taking pseudo-random value from the previous block */

			pseudo_rand = instance->memory[prev_offset].v[0];
		
		/* 1.2.2 Computing the lane of the reference block */
		ref_lane = ((pseudo_rand >> 32)) % instance->lanes;

		if ((position.pass == 0) && (position.slice == 0)) {
			/* Can not reference other lanes yet */
			ref_lane = position.lane;
		}

		/* 1.2.3 Computing the number of possible reference block within the
		* lane.
		*/
		position.index = i;
		ref_index = index_alpha(instance, &position, pseudo_rand & 0xFFFFFFFF,
			ref_lane == position.lane);

		/* 2 Creating a new block */
		ref_block =
			instance->memory + instance->lane_length * ref_lane + ref_index;
		curr_block = instance->memory + curr_offset;

			if (0 == position.pass) {
				fill_block(instance->memory + prev_offset, ref_block, curr_block, 0, instance->block_header);
				curr_block->ref_block = instance->lane_length * ref_lane + ref_index;
				curr_block->prev_block = prev_offset;
			}
			else {
				fill_block(instance->memory + prev_offset, ref_block, curr_block, 1, instance->block_header);
			}
		
	}
}

