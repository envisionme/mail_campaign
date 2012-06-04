<?php

function mail_campaign_action_subscribe_user_form($settings = array(), &$form) {
    
    $campaigns = mail_campaign_get_mail_campaigns();
    $campaign_options = array();
    foreach($campaigns as $campaign) {
      $campaign_options[$campaign->mcid] = $campaign->campaign_name;
    }
    
    
    $form['settings']['campaign'] = array(
    '#type' => 'select',
    '#title' => t('Mail Campaign'),
    '#options' => $campaign_options,
    '#default_value' => isset($settings['campaign']) ? $settings['campaign'] : '',
    '#required' => TRUE,
  );
}

function mail_campaign_condition_user_is_subscribed_form($settings = array(), &$form) {
    
    $campaigns = mail_campaign_get_mail_campaigns();
    $campaign_options = array();
    foreach($campaigns as $campaign) {
      $campaign_options[$campaign->mcid] = $campaign->campaign_name;
    }
    
    
    $form['settings']['campaign'] = array(
    '#type' => 'select',
    '#title' => t('Mail Campaign user'),
    '#options' => $campaign_options,
    '#default_value' => isset($settings['campaign']) ? $settings['campaign'] : '',
    '#required' => TRUE,
  );
}