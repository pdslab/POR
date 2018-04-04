import argparse
import glob
import math
import msvcrt as m
import os
import random
import sys

from PIL import Image

from utils import get_dir_content

FLAGS = 0


def reconstruct(patches, output_name):
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

    print("Total Patches = {}".format(len(patches)))
    print("Patch width, heigh = {}".format(patch_width))
    print("Vertical, horizontal reconstruction strides = {}".format(
        img_size))
    print("New image, (w,h) = ({},{})".format(new_w, new_h))

    # Create place holder for new iamge
    result_image = Image.new('RGB', (new_w, new_h))

    cx = 0
    cy = 0

    patch_names = list(image_patches.keys())
    if FLAGS.random_shuffle_patches:
        random.shuffle(patch_names)

    for patch_name in patch_names:
        patch = image_patches[patch_name]
        print("Concat'ing patch \'{}\' at position ({},{})".format(
            patch_name, cx, cy))
        result_image.paste(im=patch, box=(cx*patch_width, cy*patch_width))
        cx += 1

        if cx == img_size:
            cx = 0
            cy += 1

    print("Reconstructed sample, (w,h) = ({},{})".format(
        result_image.width, result_image.height))
    print("Area = {}".format(result_image.width*result_image.height))

    result_image.save(os.path.join(
        FLAGS.recon_output_dir, output_name + ".jpg"))
    if FLAGS.show_sample:
        result_image.show()

def main(_):
    patches_dir = os.listdir(FLAGS.patch_dir) 
    if not os.path.exists(FLAGS.recon_output_dir):
        os.makedirs(FLAGS.recon_output_dir)

    for patch_dir in patches_dir:
        patch_dir = os.path.join(FLAGS.patch_dir, patch_dir)
        output_name = os.path.basename(patch_dir)
        patches = list(get_dir_content(patch_dir))
        reconstruct(patches, output_name)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Process input parameters to sticher')
    parser.add_argument(
        '--patch_dir',
        type=str,
        default='D:\\Google Drive\\Data\\CatsVsDogs\\train\\train-patches\\ae\\increasing'
    )
    parser.add_argument(
        '--recon_output_dir', 
        type=str,
        default='D:\\Google Drive\\Data\\CatsVsDogs\\train\\cc-train\\ae\\increasing'
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
