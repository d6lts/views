OVERVIEW

The views module provides a flexible method for Drupal administrators to control 
how lists of content are presented. Traditionally, Drupal has hard-coded most of 
this, particularly in how taxonomy and tracker lists are formatted.

This tool is essentially a sort-of smart query builder that, given enough 
information, can build the proper query, execute it, and display the results. It 
has four modes, plus a special mode, and provides an impressive amount of 
functionality from these modes.

1) Title List.
   The simplest mode is the title list. Given the parameters set in the view, 
the module will pull up a title list and present them to the user. 

2) Table
   It can provide information in a table. The user can pick and choose what 
fields are presented, in what order they are presented, and how they are 
presented. An example of this format is the classic tracker, as in 
http://drupal.org/tracker

3) Teaser List
   The module can provide node teasers. This is actually just as simple as 
providing titles.

4) Full Nodes
   The module can provide lists of full nodes. I'm not sure how this would be 
useful, but I can imagine somebody would want it.

5) Summaries
   The fifth, special mode, exists only when the view expects arguments and does 
not get them. In this special 'summary' mode, it provides a list of what is 
available, with links to the more specific list. For example, many blogging 
sites offer archival content by month. If a view has the form of 
http://www.example.com/archive/YYYYMM, and the argument isn't specified, the 
summary would present all months available.

FEATURES

Views provides almost too many features in order to please the user.

1) Title
   The title of a view can be specified.

2) Header
   Each view can have an arbitrary amount of filtered text preceding it. This 
header can be a summary, instructions, or a description of what the user is 
seeing.

3) Paging
   Views can be set to use the pager, and each view can have its own page size. 
Views can also opt not to use the pager, and limit the number of records 
retrieved.

4) Sorting
   Views can be sorted by multiple fields, in either ascending or descending 
order.

5) Filtering
   Views can be filtered to published status, front page status, node type, 
taxonomy type and vocabulary. In the future, views can be filtered to 
arbitrary module fields.

6) Block or Page presentation
   Views can be presented either as blocks or pages; the same view can actually 
be used as both. Views can have menu entries.

7) Views can accept arbitrary arguments from the URL, and use these as filters.
   Views can accept arguments such as User IDs, Node IDs, dates and taxonomy 
terms.

Because this module is currently in Alpha Testing, the TODO list is fairly 
extensive.

NOTES

The module is implemented using only 4.6 features for now. I'd like to keep it 
this way until it's ready, and then jump it to 4.7, mostly because I don't have 
any 4.7 systems right now, and I don't want to take the time to upgrade or 
create one until there's a stable release.

I probably wrote this thing a little too quickly. Some parts are better than 
others, and it's hard for me to tell which parts are which at this point.

The module is currently named 'nodequery'. I'll rename it to 'views' (or 
something else if a better name comes up) but I've had trouble having the Right 
Name come to me.


* Half-assed Changelog (mostly for schema):

11/27/2005 added field 'handler' to view_tablefield
11/28/2005 added fields 'sortable' and 'defaultsort' to view_tablefield
