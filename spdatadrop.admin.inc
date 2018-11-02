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
    '#title' => 'Wanneer wilt u contacten synchroniseren?',
    '#default_value' => variable_get('spdatadrop_sync_type', array()),
    '#options' => array(
      'direct' => 'Direct (geen controle bevestiging inzending!)',
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
  $markup = '<table><tr><th>Webformulier</th><th>Bron (domein)</th><th><th>Formulier id</th></th><th>Cron wachtrij</th><th></th></tr>';
  $markup .= '</table>';

  return $markup;
}
