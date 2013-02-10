meshmagick -no-follow-skeleton transform -rotate=-90/1/0/0 -rotate=180/0/1/0 "./bat.mesh"
meshmagick transform -rotate=-90/1/0/0 -rotate=180/0/1/0 "./bat.skeleton"

# with renaming:
# meshmagick -no-follow-skeleton transform -rotate=-90/1/0/0 -rotate=180/0/1/0 "./bat.mesh" -- "./output.mesh"
# meshmagick transform -rotate=-90/1/0/0 -rotate=180/0/1/0 "./bat.skeleton" -- "./output.skeleton"
