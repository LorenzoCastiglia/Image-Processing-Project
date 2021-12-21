function processFile(filepath)
% Reads a nifti image identified by 'filepath' and generates two *.nii
% files, one containing a mask of the lungs and the other is the original
% image multiplied with the mask

% read the nifti image and its voxel dimensions
im = niftiread(filepath);
info = niftiinfo(filepath);
units = [info.raw.pixdim(2) info.raw.pixdim(3) info.raw.pixdim(4)];

% compute the lungs mask
mask = lungsMask(im,units);
mask = int16(mask);

% compute lungs image and write output
im = im .* mask;
niftiwrite(mask,'mask.nii');
niftiwrite(im,'lungs.nii');

end