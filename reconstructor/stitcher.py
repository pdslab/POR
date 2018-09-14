import argparse
import glob
import math
import msvcrt as m
import os
import random
import sys

from tqdm import tqdm
from PIL import Image

from pixelsort import sort_all_pixels
from utils import get_dir_content

FLAGS = 0


def reconstruct(patches, output_dir, output_name, output_format=".jpg"):
    """Reconstruct new sample from reordered patches 
    Arguments:
        patches {list} -- a list of order set of patches
    Returns:
        [image] -- reconstructed image 
    """
    image_patches = {}
    for im in patches:
        image = Image.open(im)
        image_file_basename = os.path.basename(im)
        image_patches[image_file_basename] = image
        patch_width = image.width
        patch_height = image.height

    if patch_height != patch_width:
        raise ValueError(
            "Input sample is missing a patch or so. Height and width don't match")
        sys.exit(-1)

    # calculate the new image width and height - should match original image
    img_size = math.sqrt(len(patches))
    new_h = int(patch_width*img_size)
    new_w = int(patch_width*img_size)

    #print("Total Patches = {}".format(len(patches)))
    #print("Patch width, heigh = {}".format(patch_width))
    #print("Vertical, horizontal reconstruction strides = {}".format(img_size))
    #print("New image, (w,h) = ({},{})".format(new_w, new_h))

    # Create place holder for new iamge
    result_image = Image.new('RGB', (new_w, new_h))

    cx = 0
    cy = 0

    patch_names = list(image_patches.keys())
    if FLAGS.random_shuffle_patches:
        random.shuffle(patch_names)

    # for patch_name in patch_names:
    for i in tqdm(range(len(patch_names))):
        patch_name = patch_names[i]
        patch = image_patches[patch_name]
        #patch = sort_all_pixels(patch)
        #print("Concat'ing patch \'{}\' at position ({},{})".format(patch_name, cx, cy))
        result_image.paste(im=patch, box=(cx*patch_width, cy*patch_width))
        cx += 1

        if cx == img_size:
            cx = 0
            cy += 1

    #print("Reconstructed sample, (w,h) = ({},{})".format(result_image.width, result_image.height))
    #print("Area = {}".format(result_image.width*result_image.height))

    result_image.save(os.path.join(
        output_dir, output_name + output_format))

    if FLAGS.show_sample:
        result_image.show()

def _get_categories(patch_dir):
    
    if not os.path.exists(patch_dir):
        raise ValueError("Patch directory doesn't exist. Ensure the path is correct")

    return os.listdir(patch_dir)


def main(_):
    classes =  _get_categories(FLAGS.patch_dir)
    output_dir = os.path.join(FLAGS.output_dir,FLAGS.dataset_name)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    for cateogry in classes:
        category_path = os.path.join(FLAGS.patch_dir,cateogry)
        if not os.path.exists(category_path):
            print ("ERROR-Path {} doesn't exist.SKIP".format(category_path))
        patch_sizes = os.listdir(category_path)

        for patch_size in patch_sizes:
            patch_size_path = os.path.join(category_path,patch_size)
            category_outpath = os.path.join(output_dir,FLAGS.measure,patch_size,cateogry)
            if not os.path.exists(category_outpath):
                os.makedirs(category_outpath)
            if not os.path.exists(patch_size_path):
                print ("ERROR-Path {} doesn't exist. SKIP".format(patch_size_path))
            patches_path = os.path.join(patch_size_path,FLAGS.measure)
            if not os.path.exists(patches_path):
                print ("ERROR-Patch {} doesn't exist.SKIP".format(patches_path))
            else:
                patches = list(os.listdir(patches_path))
                if len(patches) == 0:
                    print ("ERROR-Directory {} contains no patches".format(patches_path))
                
                # for patch in patches:
                for p in patches:
                    content = list(get_dir_content(os.path.join(patches_path,p)))
                    reconstruct(content,category_outpath,p,FLAGS.output_format)                

    patch_size = ["8x8","16x16","4x4"]
    measure_type = 'Average_Entropy'
    output_format = ".png"

    dataset = "E:\\DATA\cifar\\cifar10\\preprocessed\\train_patches\\"
    output = 'E:\\DATA\\cifar\\cifar10\\preprocessed\\reconstructed\\'
    output = os.path.join(output,measure_type)

    # for category in classes:
    #     for size in patch_size:
    #         patches = os.path.join(dataset,category,size,"ae\\increasing")
    #         sample_output = os.path.join(output,size,category)
    #         patches_dir = os.listdir(patches)
    #         if not os.path.exists(sample_output):
    #             os.makedirs(sample_output)

    #         for patch_dir in patches_dir:
    #             patch_dir = os.path.join(patches, patch_dir)
    #             output_name = os.path.basename(patch_dir)
    #             img_patches = list(get_dir_content(patch_dir))
    #             reconstruct(img_patches, sample_output,output_name,output_format)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Process input parameters to sticher')
    parser.add_argument(
        '--patch_dir',
        type=str,
        default='E:\\DATA\\cifar\\cifar100\\preprocessed\\train_patches'
    )
    parser.add_argument(
        '--output_dir',
        type=str,
        default='E:\\DATA\\cifar\\cifar100\\preprocessed\\reconstructed'
    )
    parser.add_argument(
        '--dataset_name',
        type=str,
        default='cifar100'
    )
    parser.add_argument(
        '--measure',
        type=str,
        default='ae/increasing'
    )
    parser.add_argument(
        '--output_format',
        type=str,
        default='.png'
    )
    parser.add_argument(
        '--random_shuffle_patches',
        type=bool,
        default=False
    )
    parser.add_argument(
        '--show_sample',
        type=bool,
        default=False
    )

    FLAGS, unparsed = parser.parse_known_args()
    main(unparsed)
