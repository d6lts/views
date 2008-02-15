<?php
// $Id$
/**
 * @file views-ui-edit-tab.tpl.php
 * Template for the primary view editing window.
 */
?>
<div class="views-display views-display-<?php print $display->id; if (!empty($display->deleted)) { print ' views-display-deleted'; }; ?>">
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
      <dl>      
        <?php // If this is the default display, add some basic stuff here. ?>
        <?php if ($default): ?>
          <dt><?php print t('View settings'); ?></dt>
          <dd class="<?php $details_class; ?>">
            <?php print $details ?>
          </dd>
        <?php endif; ?>

        <?php foreach ($categories as $category_id => $category): ?>
          <dt class="views-category-' <?php print $category_id; ?>">
            <?php print $category['title']; ?>
          </dt>
          <?php foreach ($category['data'] as $data): ?>
            <dd class="<?php print $data['class']; if (!empty($data['overridden'])) { print ' overridden'; }?>">
              <?php print $data['links'] . $data['content'] ?>    
            </dd>
          <?php endforeach; ?>
        <?php endforeach; ?>
      </dl>
    </div>
  </div>

  <?php // middle section ?>
  <div class="middle tab-section">
    <div class="inside">
      <dl>      
        <?php print $relationships; ?>
        <?php print $arguments; ?>
        <?php print $fields; ?>
      </dl>
    </div>
  </div>

  <?php // right section ?>
  <div class="right tab-section">
    <div class="inside">
      <dl>      
        <?php print $sorts; ?>
        <?php print $filters; ?>
      </dl>
    </div>
  </div>

</div>