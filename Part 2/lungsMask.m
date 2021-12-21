function lungsMask = lungsMask(image, spacings)
% This function performs the morphological detection and segmentation of
% the lungs. Some utility functions called in this script are from the
% aimutil repository
% Parameters: the image of the CT scan in nifti format and the voxel
% spacings of said image.
% Output: a nifti image representing the lungs mask 


% 1. Lungs threshold in Hounsfield Units
threshold = -150;
lungsMask = image <= threshold;

% 2. The exam table structure gets punctured to connect it to the oustide
% air, it is done by closing with a vertical prism.
% The dimensions are 1 x 10/spacings x 1 voxels
height = ceil(10 / spacings(2));
prism = ones(1, height, 1);
removeBorder = bwCloseN(lungsMask, prism);

% 3. By removing any mask component that touches the border, the air
% outside the patient is removed, including the air inside the table.
% This process is iterated for every axial slice
for i = 1 : size(image, 3)
    removeBorder(:, :, i) = imclearborder(removeBorder(:, :, i));
end
lungsMask = lungsMask & removeBorder;
clear removeBorder

% 4. Smaller air pockets are removed by opening the image with a spherical
% structuring element having diameter 10mm
diameter = 10;
siz = ceil(diameter ./ spacings);
center = 1 + (siz - 1) / 2;
ball = ballMask(siz, center, diameter / 2, spacings);
seed = bwOpenN(lungsMask, ball);

% 5. Consider the two largest connected components, which are the lungs
connectedComponents = bwconncomp(seed);
clear seed
assert(connectedComponents.NumObjects >= 2)
volumes = cell2mat(struct2cell(regionprops(connectedComponents, 'Area')));
[~, ranking] = sort(volumes, 'descend');

% 6. Morphological reconstruction to undo the effects of erosion
detect = false(size(lungsMask));
for i = 1 : 2
   detect(connectedComponents.PixelIdxList{ranking(i)}) = true;
end
lungsMask = imreconstruct(detect, lungsMask);
lungsMask = imfill(lungsMask, 'holes');

end
