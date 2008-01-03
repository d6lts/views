<?php
// $Id$
/**
 * @file views-view.tpl.php
 * Main view template
 *
 * Variables available:
 * - $header: The view header
 * - $footer: The view footer
 * - $rows: The results of the view query, if any
 * - $empty: The empty text to display if the view is empty
 * - $pager: The pager next/prev links to display, if any
 * - $filters: Exposed filter form/info to display
 * - $feed_icon: Feed icon to display, if any
 *
 * @ingroup views_templates
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
