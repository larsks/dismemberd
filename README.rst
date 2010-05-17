==========
dismemberd
==========

Dismemberd uses the Corosync_ "closed process group" (CPG) API to
dynamically maintain a list of hosts in a cluster.  On membership changes
it will write out a list of cluster members to files and optionally execute
a user-defined action.

I wrote this originally to maintain dsh/pdsh compatible group lists for a
dynamic cluster environment.

.. _corosync: http://www.openais.org/doku.php

Usage
=====

``dismemberd`` [ ``-g`` *group* [ ``-g`` *group* ... ] ]
[ ``-d`` *group_list_dir* ] [ ``-x`` *script* ] [ ``-ns`` ]

Options
=======

-g group              Join group named *group*.  Dismemberd may join multiple
                      groups; a file will be maintained for each group.

-d group_list_dir     Group files will be created in this directory.

-x script             Execute script on membership change.  See
                      ``Script environment``, below, for more information.

-n                    Do not update files on membership changes.

-s                    Log to syslog instead of stderr.

Script environment
==================

Scripts executed with the ``-x`` option have access to the following
environment variables:

``DMD_GROUP_LIST_DIR``
  Path to the directory in which *dismemberd* is writing group membership
  files.

``DMD_NUM_GROUPS``
  Number of groups of which *dismemberd* is a member.

``DMD_GROUP_1`` ... ``DMD_GROUP_N``
  Names of groups of which *dismemberd* is a member.

You could use something like the following snippet of bash_ code to access
the group names::

  for (( i=0; i<$DMD_NUM_GROUPS; i++ )) {
    eval "grpname=\"\$DMD_GROUP_$i\""
    echo "Members of group $grpname:"
    cat "$DMD_GROUP_LIST_DIR/$grpname"
  }

.. _bash: http://www.gnu.org/software/bash/

Requirements
============

This project requires the following third-party libraries:

- corosync (tested with 1.2.1)
- glib (tested with 2.22.5)

Author
======

Lars Kellogg-Stedman <lars@oddbit.com>

