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
    '#title' => 'Geheime sleutel webformsync',
    '#description' => 'This secret should match the secret set in spwebformsync modules pushing to spdatadrop.',
    '#default_value' => variable_get('spdatadrop_secret', ''),
  );

  $form['spdatadrop_sync_settings']['spdatadrop_secret_unsubscribe'] = array(
    '#type' => 'textfield',
    '#title' => 'Geheime sleutel uitschrijf link',
    '#description' => 'This secret should match the unsubscribe secret set in spwebformsync modules pushing to spdatadrop.',
    '#default_value' => variable_get('spdatadrop_secret_unsubscribe', ''),
  );

  $form['spdatadrop_sync_settings']['spdatadrop_allowed_ips'] = array(
    '#type' => 'textarea',
    '#title' => 'IP adressen die data  mogen droppen',
    '#default_value' => variable_get('spdatadrop_allowed_ips', ''),
    '#description' => 'Één per regel',
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
  // Sync.
  $number = variable_get('spdatadrop_cron_number', 0);
  $number_pushed = 0;
  // First sync all form submissions.
  $sync_ids = spdatadrop_sync_submissions($number);
  $to_go = $number - count($sync_ids);
  if ($to_go > 0) {
    $number_pushed = spdatadrop_unsubscribe_submissions($to_go);
  }

  // Process callbacks.
  spdatadrop_process_callbacks($number);


  // Feedback.
  if (!empty($sync_ids)) {
    $text = '<div id="sync_now"><p>Formulier inzendingen die gesynced zijn (CiviCRM contact ids): ' . implode(', ', $sync_ids) . '</p>';
  }
  else {
    $text = '<div id="sync_now"><p>Geen te syncen formulier inzendingen gevonden.</p>';
  }
  $text .= '<p>Aantal verwerkte uitschrijvingen: ' . $number_pushed . '</p>';
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

  $query = "SELECT source_domain, source_title, source_id, state, callback_path, callback_state FROM {spdatadrop_syncstate}";
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
      // Callback processed.
      if (!isset($tabledata[$row['source_domain']][$row['source_id']]['callback_processed']['quantity'])) {
        $tabledata[$row['source_domain']][$row['source_id']]['callback_processed']['quantity'] = 0;
      }
      if (!empty($row['callback_path']) && $row['callback_state'] === '1') {
        $tabledata[$row['source_domain']][$row['source_id']]['callback_processed']['quantity']++;
      }
      // Callback error.
      if (!isset($tabledata[$row['source_domain']][$row['source_id']]['callback_error']['quantity'])) {
        $tabledata[$row['source_domain']][$row['source_id']]['callback_error']['quantity'] = 0;
      }
      if (!empty($row['callback_path']) && $row['callback_state'] === '3') {
        $tabledata[$row['source_domain']][$row['source_id']]['callback_error']['quantity']++;
      }
    }
  }

  // Create header.
  $tableheader = array();
  $tableheader[] = 'Titel';
  foreach ($states as $state_id => $state_title) {
    $tableheader[] = $states[$state_id];
  }
  $tableheader[] = 'Callback processed';
  $tableheader[] = 'Callback error';

  // Create tables content.
  $output = '';
  if (!empty($tabledata)) {
    $tablerows = array();
    $row_id = 0;
    foreach ($tabledata as $domain => $domaindata) {
      // Add domain header.
      $tablerows[$row_id] = array(
        'data' => array(
          array(
            'data' => '<p><strong>' . $domain . '</strong></p>',
            'colspan' => count($states) + 3,
          ),
        ),
        'no_striping' => TRUE,
        'class' => array('odd'),
      );
      $row_id++;
      foreach ($domaindata as $source_id => $source_data) {
        $tablerows[$row_id]['no_striping'] = TRUE;
        $tablerows[$row_id]['data']['title'] = $source_data['title'];
        foreach ($states as $state_id => $state_title) {
          if (empty($tabledata[$domain][$source_id]['state_data'][$state_id]['quantity'])) {
            $tablerows[$row_id]['data'][$state_title] = 0;
          }
          else {
            $tablerows[$row_id]['data'][$state_title] = $tabledata[$domain][$source_id]['state_data'][$state_id]['quantity'];
          }
        }
        $tablerows[$row_id]['data']['callback_processed'] = $tabledata[$domain][$source_id]['callback_processed']['quantity'];
        $tablerows[$row_id]['data']['callback_error'] = $tabledata[$domain][$source_id]['callback_error']['quantity'];
        $row_id++;
      }
    }
    $output = theme('table', array('header' => $tableheader, 'rows' => $tablerows));
  }
  return $output;
}

/**
 * Provide overview of unsubscriptions.
 */
function spdatadrop_unsubscriptions() {
  $ouput = '';
  $states = array_flip(spdatadrop_get_states());
  // Get data.
  $tabledata = array();

  $query = "SELECT uns.state, unsubscribe, is_opt_out, do_not_phone, do_not_sms, source_domain, source_title, source_id FROM {spdatadrop_unsubscribe} uns LEFT JOIN {spdatadrop_syncstate} syn ON uns.secret = syn.secret";
  $result = db_query($query);
  if ($result) {
    while ($row = $result->fetchAssoc()) {
      // Set title.
      $title = empty($row['source_title']) ? 'Wacht op webform sync' : $row['source_title'] . ' (' . $row['source_id'] . ')';
      if (!isset($tabledata[$row['source_domain']][$row['source_id']]['title'])) {
        $tabledata[$row['source_domain']][$row['source_id']]['title'] = $title;
      }
      // Count total.
      if (!isset($tabledata[$row['source_domain']][$row['source_id']]['total']['quantity'])) {
        $tabledata[$row['source_domain']][$row['source_id']]['total']['quantity'] = 0;
      }
      $tabledata[$row['source_domain']][$row['source_id']]['total']['quantity']++;
      // Set options.
      $options = array('unsubscribe', 'is_opt_out', 'do_not_phone', 'do_not_sms');
      foreach ($options as $option) {
        if (!isset($tabledata[$row['source_domain']][$row['source_id']][$option]['quantity'])) {
          $tabledata[$row['source_domain']][$row['source_id']][$option]['quantity'] = 0;
        }
        if ($row[$option]) {
          $tabledata[$row['source_domain']][$row['source_id']][$option]['quantity']++;
        }
      }
      // Set states.
      if (!isset($tabledata[$row['source_domain']][$row['source_id']]['state_data'][$row['state']]['quantity'])) {
        $tabledata[$row['source_domain']][$row['source_id']]['state_data'][$row['state']]['quantity'] = 0;
      }
      $tabledata[$row['source_domain']][$row['source_id']]['state_data'][$row['state']]['quantity']++;
    }
  }

  // Create header.
  $tableheader = array();
  $tableheader[] = 'Titel';
  $tableheader[] = 'Totaal aantal';
  $tableheader[] = 'Uitschrijven groepen';
  $tableheader[] = 'Geen mail';
  $tableheader[] = 'Niet bellen';
  $tableheader[] = 'Niet sms\'en';
  foreach ($states as $state_id => $state_title) {
    $tableheader[] = $states[$state_id];
  }

  // Create tables content.
  $output = '';
  if (!empty($tabledata)) {
    $tablerows = array();
    $row_id = 0;
    foreach ($tabledata as $domain => $domaindata) {
      // Add domain header.
      $tablerows[$row_id] = array(
        'data' => array(
          array(
            'data' => '<p><strong>' . $domain . '</strong></p>',
            'colspan' => count($states) + 6,
          ),
        ),
        'no_striping' => TRUE,
        'class' => array('odd'),
      );
      $row_id++;
      foreach ($domaindata as $source_id => $source_data) {
        $tablerows[$row_id]['no_striping'] = TRUE;
        $tablerows[$row_id]['data']['title'] = $source_data['title'];
        if (empty($tabledata[$domain][$source_id]['total']['quantity'])) {
          $tablerows[$row_id]['data']['total'] = 0;
        }
        else {
          $tablerows[$row_id]['data']['total'] = $tabledata[$domain][$source_id]['total']['quantity'];
        }
        foreach ($options as $option) {
          if (empty($tabledata[$domain][$source_id][$option]['quantity'])) {
            $tablerows[$row_id]['data'][$option] = 0;
          }
          else {
            $tablerows[$row_id]['data'][$option] = $tabledata[$domain][$source_id][$option]['quantity'];
          }
        }
        foreach ($states as $state_id => $state_title) {
          if (empty($tabledata[$domain][$source_id]['state_data'][$state_id]['quantity'])) {
            $tablerows[$row_id]['data'][$state_title] = 0;
          }
          else {
            $tablerows[$row_id]['data'][$state_title] = $tabledata[$domain][$source_id]['state_data'][$state_id]['quantity'];
          }
        }
        $row_id++;
      }
    }
    $output = theme('table', array('header' => $tableheader, 'rows' => $tablerows));
  }
  return $output;
}
