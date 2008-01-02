<?php
// $Id$
/**
 * @file views-view-rows.tpl.php
 * Default simple view template to display a list of summary lines
 */
?>
<div class="item-list">
  <ul>
  <?php foreach ($rows as $row): ?>
    <li><?php print $row?></li>
  <?php endforeach; ?>
  </ul>
</div>
