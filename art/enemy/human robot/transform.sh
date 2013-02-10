meshmagick -no-follow-skeleton transform -rotate=-90/1/0/0 -rotate=180/0/1/0 "./humanRobot.mesh"
meshmagick transform -rotate=-90/1/0/0 -rotate=180/0/1/0 "./humanRobot.skeleton"

# with renaming:
# meshmagick -no-follow-skeleton transform -rotate=-90/1/0/0 -rotate=180/0/1/0 "./humanRobot.mesh" -- "./output.mesh"
# meshmagick transform -rotate=-90/1/0/0 -rotate=180/0/1/0 "./humanRobot-Armature.skeleton" -- "./output.skeleton"
