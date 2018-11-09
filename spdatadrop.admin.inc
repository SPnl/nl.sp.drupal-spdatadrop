<?php

/**
 * @file
 * Admin interface.
 */

/**
 * Settings form.
 */
function spdatadrop_settings_form($form, &$form_state) {
  // Synchronization type.
  if (empty(variable_get('spdatadrop_sync_type', FALSE))) {
    variable_set('spdatadrop_sync_type', 'cron');
  }
  $form['spdatadrop_sync_settings'] = array(
    '#type' => 'fieldset',
    '#title' => 'Sync instellingen',
  );

  $form['spdatadrop_sync_settings']['spdatadrop_secret'] = array(
    '#type' => 'textfield',
    '#title' => 'Geheime sleutel',
    '#description' => 'This secret should match the secret set in spwebformsync modules pushing to spdatadrop.',
    '#default_value' => variable_get('spdatadrop_secret', ''),
  );

  $form['spdatadrop_sync_settings']['spdatadrop_sync_type'] = array(
    '#type' => 'radios',
    '#title' => 'Hoe wilt u contacten synchroniseren?',
    '#default_value' => variable_get('spdatadrop_sync_type', array()),
    '#options' => array(
      'cron' => 'Cron',
      'manual' => 'Handmatig',
    ),
    '#required' => TRUE,
  );

  // Max sync items per cron.
  $form['spdatadrop_sync_settings']['spdatadrop_cron_number'] = array(
    '#type' => 'select',
    '#title' => 'Hoeveel inzendingen moeten per keer gesynchroniseerd worden?',
    '#default_value' => variable_get('spdatadrop_cron_number', 0),
    '#options' => range(0, 30, 1),
    '#states' => array(
      'invisible' => array(
        ':input[name="spdatadrop_sync_type"]' => array('value' => 'direct'),
      ),
    ),
  );

  $form['spdatadrop_sync_settings']['spdatadrop_sync_now'] = array(
    '#type' => 'button',
    '#value' => 'Nu synchroniseren',
    '#name' => 'sync_now',
    '#states' => array(
      'visible' => array(
        ':input[name="spdatadrop_sync_type"]' => array('value' => 'manual'),
      ),
    ),
    '#ajax' => array(
      'wrapper' => 'sync_now',
      'callback' => 'spdatadrop_form_sync',
    ),
    '#prefix' => '<div id="sync_now">',
    '#suffix' => '</div>',
  );

  return system_settings_form($form);
}

/**
 * Sync.
 */
function spdatadrop_form_sync($form, &$form_state) {
  $number = variable_get('spdatadrop_cron_number', 0);
  $processed = spdatadrop_push_contacts($number);
  if (!empty($processed)) {
    $text = '<div id="sync_now"><p>Contacten die gesynced zijn (CiviCRM contact ids): ' . implode(', ', $processed) . '</p>';
  }
  else {
    $text = '<div id="sync_now"><p>Geen inzendingen gevonden.</p>';
  }
  $form['spdatadrop_sync_settings']['spdatadrop_sync_now']['#prefix'] = $text;
  return $form['spdatadrop_sync_settings']['spdatadrop_sync_now'];
}

/**
 * Provide overview of syncs.
 */
function spdatadrop_overview() {
  $ouput = '';
  $states = array_flip(spdatadrop_get_states());
  // Get data.
  $tabledata = array();

  $query = "SELECT source_domain, source_title, source_id, state FROM {spdatadrop_syncstate}";
  $result = db_query($query);
  if ($result) {
    while ($row = $result->fetchAssoc()) {
      if (!isset($tabledata[$row['source_domain']][$row['source_id']]['title'])) {
        $tabledata[$row['source_domain']][$row['source_id']]['title'] = $row['source_title'] . ' (' . $row['source_id'] . ')';
      }
      if (!isset($tabledata[$row['source_domain']][$row['source_id']]['state_data'][$row['state']]['quantity'])) {
        $tabledata[$row['source_domain']][$row['source_id']]['state_data'][$row['state']]['quantity'] = 0;
      }
      $tabledata[$row['source_domain']][$row['source_id']]['state_data'][$row['state']]['quantity']++;
    }
  }

  // Create header.
  $tableheader = array();
  $tableheader[] = 'Titel';
  foreach ($states as $state_id => $state_title) {
    $tableheader[] = $states[$state_id];
  }

  // Create tables content.
  $output = '';
  if (!empty($tabledata)) {
    foreach ($tabledata as $domain => $domaindata) {
      $output .= '<p><strong>' . $domain . '</strong></p>';
      foreach ($domaindata as $source_id => $source_data) {
        $tablerows[$source_id] = array(
          'title' => $source_data['title'],
        );
        foreach ($states as $state_id => $state_title) {
          if (empty($tabledata[$domain][$source_id]['state_data'][$state_id]['quantity'])) {
            $tablerows[$source_id][$state_title] = 0;
          }
          else {
            $tablerows[$source_id][$state_title] = $tabledata[$domain][$source_id]['state_data'][$state_id]['quantity'];
          }
        }
      }
      $output .= theme('table', array('header' => $tableheader, 'rows' => $tablerows));
    }
  }
  return $output;
}
