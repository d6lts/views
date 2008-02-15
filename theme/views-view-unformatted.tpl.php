<?php
// $Id$
/**
 * @file views-view-unformatted.tpl.php
 * Default simple view template to display a list of rows.
 *
 * @ingroup views_templates
 */
?>
<?php foreach ($rows as $row): ?>
  <?php print $row ?>
<?php endforeach; ?>