
TEMPLATE = subdirs
# Needed to ensure that things are built right, which you have to do yourself :(
CONFIG += ordered

# All the projects in your application are sub-projects of your solution
SUBDIRS = VBE \
	  VBE-Scenegraph \
          game


# Use .depends to specify that a project depends on another.
VBE-Scenegraph.depends = VBE
game.depends = VBE VBE-Scenegraph

OTHER_FILES += \
        common.pri
