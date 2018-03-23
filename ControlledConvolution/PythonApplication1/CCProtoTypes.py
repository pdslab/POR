import cv2

img1 = cv2.imread("C:\\Users\\Henok\\Google Drive\\Thesis_Implementation\\patchs\\image data\\0.0_256.256.bmp")
img2 = cv2.imread("C:\\Users\\Henok\\Google Drive\\Thesis_Implementation\\patchs\\image data\\0.256_256.512.bmp")

norm_l1 = cv2.norm(img1, img2, cv2.NORM_L1)
norm_l2 = cv2.norm(img1, img2, cv2.NORM_L2)

print ("l1 norm {}", str(norm_l1))
print ("l2 norm {}", str(norm_l2))
