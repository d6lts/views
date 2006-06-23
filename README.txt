// $Id$

OVERVIEW

The views module provides a flexible method for Drupal administrators to control 
how lists of content are presented. Traditionally, Drupal has hard-coded most of 
this, particularly in how taxonomy and tracker lists are formatted.

This tool is essentially a sort-of smart query builder that, given enough 
information, can build the proper query, execute it, and display the results. It 
has four modes, plus a special mode, and provides an impressive amount of 
functionality from these modes.

INSTALLING VIEWS

In 4.7, Views can be installed simply by activating the module, thanks to the
new module installation features.

Please note that this is now TWO modules -- views.module and views_ui.module
DOCUMENTATION

Documentation is now available at http://drupal.org/handbook/modules/views.
The documentation files in the views package have been removed since they
are even more out of date.

As of 3/16/2006, the user documentation is UP TO DATE. The API documentation
is somewhat out of date, and the theming documentation is very out of date.
I hope to correct this in the very near future.

-------------- IMPORTANT --------------------------------------------------
-------------- IMPORTANT --------------------------------------------------

If you update Views or any module that uses Views, you MUST MUST MUST
go and resubmit the admin/modules page. Views caches information provided
by modules, and this information MUST be re-cached whenever an update
is performed. 

DRUPAL CANNOT TELL AUTOMATICALLY IF YOU HAVE UPDATED CODE. Therefore you
must go and submit this page.

Hopefully in 4.8 this won't be an issue.
