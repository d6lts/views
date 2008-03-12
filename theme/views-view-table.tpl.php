<?php
// $Id$
/**
 * @file views-view-table.tpl.php
 * Template to display a view as a table.
 *
 * @ingroup views_templates
 */
?>
<table>
  <thead>
    <tr>
      <?php foreach ($header as $field => $data): ?>
        <?php // @todo click sort styling and link ?>
        <th class="views-field-<?php print $fields[$field]; ?>">
          <?php print $data['label']; ?>
        </th>
      <?php endforeach ?>
    </tr>
  </thead>
  <tbody>
    <?php foreach ($rows as $count => $row): ?>
      <tr class="<?php print ($count % 2 == 0) ? 'even' : 'odd';?>">
        <?php foreach ($row as $field => $data): ?>
          <td class="views-field-<?php print $fields[$field]; ?>">
            <?php print $data; ?>
          </td>
        <?php endforeach; ?>
      </tr>
    <?php endforeach; ?>
  </tbody>
</table>
