## 
EXPLORING DEEP LEARNING USING INFORMATION THEORY TOOLS AND PATCH ORDERING 


Henok Ghebrechrisots, Gita Alaghband, PhD


ABSTRACT

We present a framework for automatically ordering image patches that enables in-depth analysis of dataset relationship to learnability of a classification task using convolutional neural network. An image patch is a group of pixels residing in a continuous area contained in the sample. Our preliminary experimental results show that an informed smart shuffling of patches at a sample level can expedite training by exposing important features at early stages of training. In addition, we conduct systematic experiments and provide evidence that CNN’s generalization capabilities do not correlate with human recognizable features present in training samples. We utilized the framework not only to show that spatial locality of features within samples do not correlate with generalization, but also to expedite convergence while achieving similar generalization performance. Using multiple network architectures and datasets, we show that ordering image regions using mutual information measure between adjacent patches, enables CNNs to converge in a third of the total steps required to train the same network without patch ordering.
