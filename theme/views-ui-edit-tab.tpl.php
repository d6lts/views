<?php
// $Id$
/**
 * @file views-ui-edit-tab.tpl.php
 * Template for the primary view editing window.
 */
?>
<div class="clear-block views-display views-display-<?php print $display->id; if (!empty($display->deleted)) { print ' views-display-deleted'; }; ?>">
  <?php // top section ?>
  <div class="top">
    <div class="inside">
      <?php print $remove ?>
      <span class="display-title">
        <?php print $title; ?>
      </span>
      <span class="display-description">
        <?php print $description; ?>
      </span>
    </div>
  </div>

  <?php // left section ?>
  <div class="left tab-section">
    <div class="inside">
      <?php // If this is the default display, add some basic stuff here. ?>
      <?php if ($default): ?>
        <div class="views-category">
          <div class="views-category-title"><?php print t('View settings'); ?></div>
          <div class="<?php $details_class; if (!empty($details_changed)) { print ' changed'; }?>">
            <?php print $details ?>
          </div>
        </div>
      <?php endif; ?>

      <?php foreach ($categories as $category_id => $category): ?>
        <div class="views-category">
        <div class="views-category-title views-category-<?php print $category_id; ?>">
          <?php print $category['title']; ?>
        </div>
        <?php foreach ($category['data'] as $data): ?>
          <div class="<?php print $data['class']; if (!empty($data['overridden'])) { print ' overridden'; } if (!empty($data['changed'])) { print ' changed'; }?>">
            <?php print $data['links'] . $data['content'] ?>
          </div>
        <?php endforeach; ?>
        </div>
      <?php endforeach; ?>
    </div>
  </div>

  <?php // middle section ?>
  <div class="middle tab-section">
    <div class="inside">
      <div class="views-category">
        <?php print $relationships; ?>
      </div>
      <div class="views-category">
        <?php print $arguments; ?>
      </div>
      <?php if (!empty($fields)): ?>
        <div class="views-category">
          <?php print $fields; ?>
        </div>
      <?php endif; ?>
    </div>
  </div>

  <?php // right section ?>
  <div class="right tab-section">
    <div class="inside">
      <div class="views-category">
        <?php print $sorts; ?>
      </div>
      <div class="views-category">
        <?php print $filters; ?>
      </div>
    </div>
  </div>
  <span class="clear"></span>

</div>