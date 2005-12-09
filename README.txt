OVERVIEW

The views module provides a flexible method for Drupal administrators to control 
how lists of content are presented. Traditionally, Drupal has hard-coded most of 
this, particularly in how taxonomy and tracker lists are formatted.

This tool is essentially a sort-of smart query builder that, given enough 
information, can build the proper query, execute it, and display the results. It 
has four modes, plus a special mode, and provides an impressive amount of 
functionality from these modes.

1) List View
   The view can present the nodes as a simple list of fields, with each field
   encapsulated in a <div> tag, with an id based upon the field name. CSS can
   then be used to style the list. Which fields are presented and in what order
   is configurable within the view.

2) Table View
   It can provide information in a table. The user can pick and choose what 
   fields are presented, in what order they are presented, and how they are 
   presented. An example of this format is the classic tracker, as in 
   http://drupal.org/tracker

3) Teaser List
   The module can provide node teasers, similar to the default front page
   or taxonomy/term.

4) Full Nodes
   The module can provide lists of full nodes. I'm not sure how this would be 
   useful. This could be useful if the list were limited to a single node.

5) Random Node
   A view can be set to return a random node from a view, and display a teaser
   or a full node.

6) Summaries
   The last, special mode, exists only when the view expects arguments and does 
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

7) Arguments
   Views can accept arbitrary arguments from the URL, and use these as filters.
   Views can accept arguments such as User IDs, Node IDs, dates and taxonomy 
   terms.

8) Themeing
   Each view can have its own theme, in the form of theme_view_VIEWNAME(),
   allowing theme designers and site administrators to pick and choose which
   views need additional formatting. The theme will receive the nodes already
   loaded from the query, along with any information needed for things such
   as table headers.

9) Module Developer API
   Views supports an API for module developers, allowing them to expose their
   tables and fields for viewing, sorting and filtering.

10) Default Views
    Both modules and the system itself can provide default views that can be
    utilized in various ways. These default views can be overriden by the
    admin and recustomized in any way desired. As this module sees wider
    use, the hope is that core modules will provide their node-list interfaces
    as views so that administrators can more easily customize how those views
    are presented.

NOTES

The module is implemented using only 4.6 features for now. I'd like to keep it 
this way until it's ready, and then jump it to 4.7, mostly because I don't have 
any 4.7 systems right now, and I don't want to take the time to upgrade or 
create one until there's a stable release.

I probably wrote this thing a little too quickly. Some parts are better than 
others, and it's hard for me to tell which parts are which at this point.

database.inc has a minor bug that makes from_unixtime not work:

(line 246 in my version but it's kind of old).
    $query = preg_replace('|FROM[^[:upper:]/,]+|','\0 '. $join .' ', $query);
should be
    $query = preg_replace('|FROM[^[:upper:]_/,]+|','\0 '. $join .' ', $query);


* Half-assed Changelog (mostly for schema):

11/27/2005 added field 'handler' to view_tablefield
11/28/2005 added fields 'sortable' and 'defaultsort' to view_tablefield
11/28/2005 changed view_argument type from int(2) to varchar(25) for greater flexibility
11/30/2005 changed filter table and view table significantly. Probably sort table too.
12/5/2005 Add options varchar(255) to both sort and filter tables.
12/7/2005 Add menu_title to view_view and title to view_argument
12/8/2005 Added options varchar(255) to view_argument as well.