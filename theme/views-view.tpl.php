<?php 
// $Id$
/**
 * @file views-view.tpl.php
 * Main view template
 *
 */
?>
<div class="view view-<?php print $css_name; ?>">
  <?php if ($header): ?>
    <div class="view-header">
      <?php print $header; ?>
    </div>
  <?php endif; ?>

  <?php if ($filters): ?>
    <div class="view-filters">
      <?php print $filters; ?>
    </div>
  <?php endif; ?>

  <?php if ($rows): ?>
    <div class="content">
      <?php print $rows; ?>
    </div>
  <?php elseif ($empty): ?>
    <div class="view-empty">
      <?php print $empty; ?>
    </div>
  <?php endif; ?>

  <?php if ($pager): ?>
    <?php print $pager; ?>
  <?php endif; ?>

  <?php if ($footer): ?>
    <div class="view-footer">
      <?php print $footer; ?>
    </div>
  <?php endif; ?>

  <?php if ($feed_icon): ?>
    <div class="feed-icon">
      <?php print $feed_icon; ?>
    </div>
  <?php endif; ?>

</div> <?php // class view ?>
