/*
 * Copyright (C) 2015, Nils Moehrle
 * TU Darmstadt - Graphics, Capture and Massively Parallel Computing
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD 3-Clause license. See the LICENSE.txt file for details.
 */

#include <set>
#include <list>
#include <iostream>
#include <fstream>

#include <util/timer.h>
#include <mve/image_tools.h>

#include "defines.h"
#include "settings.h"
#include "histogram.h"
#include "texture_patch.h"
#include "texture_atlas.h"

#define MAX_TEXTURE_SIZE (32 * 1024)
#define PREF_TEXTURE_SIZE (4 * 1024)
#define MIN_TEXTURE_SIZE (256)

TEX_NAMESPACE_BEGIN

/**
  * Heuristic to calculate an appropriate texture atlas size.
  * @warning asserts that no texture patch exceeds the dimensions
  * of the maximal possible texture atlas size.
  */
unsigned int
calculate_texture_size(std::list<TexturePatch::ConstPtr> const & texture_patches) {
    unsigned int size = MAX_TEXTURE_SIZE;

    while (true) {
        unsigned int total_area = 0;
        unsigned int max_width = 0;
        unsigned int max_height = 0;
        unsigned int padding = std::min(size >> 7, 32U);

        for (TexturePatch::ConstPtr texture_patch : texture_patches) {
            unsigned int width = texture_patch->get_width() + 2 * padding;
            unsigned int height = texture_patch->get_height() + 2 * padding;

            max_width = std::max(max_width, width);
            max_height = std::max(max_height, height);

            unsigned int area = width * height;
            unsigned int waste = area - texture_patch->get_size();

            /* Only consider patches where the information dominates padding. */
            if (static_cast<double>(waste) / texture_patch->get_size() > 1.0) {
                /* Since the patches are sorted by size we can assume that only
                 * few further patches will contribute to the size and break. */
                break;
            }

            total_area += area;
        }

        if (size <= MIN_TEXTURE_SIZE) {
            return MIN_TEXTURE_SIZE;
        }

        if (max_height < size / 2 && max_width < size / 2 &&
            static_cast<double>(total_area) / (double(size) * size) < 0.2) {
            size = size / 2;
            continue;
        }

        return size;
    }
}

bool comp(TexturePatch::ConstPtr first, TexturePatch::ConstPtr second) {
    return first->get_size() > second->get_size();
}

void
generate_texture_atlases(std::vector<TexturePatch::Ptr> * orig_texture_patches,
    Settings const & settings, std::vector<TextureAtlas::Ptr> * texture_atlases) {

    std::list<TexturePatch::ConstPtr> texture_patches;
    while (!orig_texture_patches->empty()) {
        TexturePatch::Ptr texture_patch = orig_texture_patches->back();
        orig_texture_patches->pop_back();

        if (settings.tone_mapping != TONE_MAPPING_NONE) {
            mve::image::gamma_correct(texture_patch->get_image(), 1.0f / 2.2f);
        }

        texture_patches.push_back(texture_patch);
    }

    std::cout << "\tSorting texture patches... " << std::flush;
    /* Improve the bin-packing algorithm efficiency by sorting texture patches
     * in descending order of size. */
    texture_patches.sort(comp);
    std::cout << "done." << std::endl;

    std::size_t const total_num_patches = texture_patches.size();
    std::size_t remaining_patches = texture_patches.size();
    std::ofstream tty("/dev/tty", std::ios_base::out);

    unsigned int texture_size = calculate_texture_size(texture_patches);
    TextureAtlas::Ptr texture_atlas = TextureAtlas::create(texture_size);
    while (remaining_patches > 0) {
        /* Try to insert each of the texture patches into the texture atlas. */
        std::list<TexturePatch::ConstPtr>::iterator it = texture_patches.begin();
        for (; it != texture_patches.end();) {
            std::size_t done_patches = total_num_patches - remaining_patches;
            int precent = static_cast<float>(done_patches)
                / total_num_patches * 100.0f;
            if (total_num_patches > 100
                && done_patches % (total_num_patches / 100) == 0) {

                tty << "\r\tWorking on atlas " << precent << "%... " << std::flush;
            }

            if (texture_atlas->insert(*it)) {
                ++it;
                remaining_patches -= 1;
            } else {
		/* Texture atlas was too small, try again. */
		texture_size *= 2;
		if (texture_size > MAX_TEXTURE_SIZE) {
			std::cerr << "Exceeded maximum texture size (" << MAX_TEXTURE_SIZE << ")." << std::endl;
			std::exit(EXIT_FAILURE);
		}

		remaining_patches = texture_patches.size();
		it = texture_patches.begin();
		texture_atlas = TextureAtlas::create(texture_size);
		break;
            }
        }
    }

    texture_atlases->push_back(texture_atlas);
    texture_atlas->finalize();
}

TEX_NAMESPACE_END
