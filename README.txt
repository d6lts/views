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

Enable Views and Views UI modules.  You can disable Views UI module after you
have finished your site's setup.

DOCUMENTATION

Documentation is now available at http://drupal.org/handbook/modules/views.


-------------- IMPORTANT --------------------------------------------------
-------------- IMPORTANT --------------------------------------------------

If you update Views or any module that uses Views, you MUST MUST MUST
go and resubmit the admin/modules page. Views caches information provided
by modules, and this information MUST be re-cached whenever an update
is performed. 

DRUPAL CANNOT TELL AUTOMATICALLY IF YOU HAVE UPDATED CODE. Therefore you
must go and submit this page.

Hopefully in 4.8 this won't be an issue.

-------------- UPDATING VIEWS ---------------------------------------------

If you're updating from 4.6 to 4.7+, this is important:
  Views Will Not Update Properly.

I'm sorry, but there is no upgrade path from 4.6 to 4.7. In order to
achieve this result, you need to first completely unstall Views. Before
you do this you may want to write down your view information.

Then uninstall views; there are instructions on the views module handbook
page on drupal.org -- http://drupal.org/handbook/modules/views.

Once you have it completely uninstalled, you may then install the new
Views for 4.7, via Drupal's automatic installer. 
