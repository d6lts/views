<?php
// $Id$
/**
 * @file views-view-list.tpl.php
 * Default simple view template to display a list of rows.
 *
 * - $options['type'] will either be ul or ol.
 * @ingroup views_templates
 */
?>
<div class="item-list">
  <<?php print $options['type']; ?>>
    <?php foreach ($rows as $row): ?>
      <li><?php print $row ?></li>
    <?php endforeach; ?>
  </<?php print $options['type']; ?>>
</div>