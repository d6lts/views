<?php
// $Id$
/**
 * @file views-view-fields.tpl.php
 * Default simple view template to all the fields as a row.
 *
 * @ingroup views_templates
 */
?>
<?php foreach ($fields as $name => $class): ?>
  <div class="<?php print $class ?>">
    <?php // this is indirection; $name contains the name of the variable. ?>
    <?php print $$name; ?>
  </div>
<?php endforeach; ?>
