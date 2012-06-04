<?php
/**
 * @file
 * Mail Campaign Rules Integration.
 */

function mail_campaign_rules_event_info() {
 return array(
    'mail_campaign_user_subscribed' => array(
      'label' => t('User has been subscribed to a campaign.'),
      'arguments' => array(
        'user' => array('type' => 'user', 'label' => t('User')),
        'campaign' => array('type' => 'value', 'label' => t('Campaign')),
      ),
      'module' => 'Mail Campaign',
    ),
  );
} 
 
 
function mail_campaign_rules_action_info() {
 return array(
    'mail_campaign_action_subscribe_user' => array(
      'label' => t('Subscribe user to a campaign'),
      'arguments' => array(
        'user' => array('type' => 'user', 'label' => t('User')),
        'campaign' => array('type' => 'value', 'label' => t('Campaign')),
      ),
      'module' => 'Mail Campaign',
    ),
  );
}

/**
 * Implementation of hook_rules_condition_info().
 */

function mail_campaign_rules_condition_info() {
  return array(
    'mail_campaign_condition_user_is_subscribed' => array(
      'label' => t('User is subscribed to a specific campaign'),
      'arguments' => array(
        'user' => array('type' => 'user', 'label' => t('User')),
        'campaign' => array('type' => 'value', 'label' => t('Campaign')),
      ),
      'module' => 'Mail Campaign',
    ),
  );  
}



function mail_campaign_action_subscribe_user($user, $campaign) {
  $campaign = mail_campaign_load($campaign);
  if(mail_campaign_subscribe_user($user->uid, $campaign->mcid)) {
    watchdog('mail campaign', t('[Rules] Subscribed user %user to %campaign', array('%user' => $user->name, '%campaign' => $campaign->campaign_name)));
  }    
}



function mail_campaign_condition_user_is_subscribed($user, $campaign){
  $campaign = mail_campaign_load($campaign);
  $check = db_result(db_query('select join_time from {mail_campaign_user} where uid = %d and mcid = %d', array($user->uid, $campaign->mcid)));
  if ($check) {
    return true;
  }
  else {
    return false;
  }
    
}