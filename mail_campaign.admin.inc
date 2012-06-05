<?php

/**
 * @file
 * Contains administrative pages for creating, editing, and deleting mail campaigns.
 */

/**
 * Mail Campaign administration page. Display a list of existing Mail Campaigns.
 */
function mail_campaign_admin_page() {
  
  $mail_campaigns = mail_campaign_get_mail_campaigns();
  return theme('mail_campaign_admin_page', $mail_campaigns);
}

/**
 * Mail Campaign email administration page. Display a list of emails in Campaign.
 */
function mail_campaign_email_admin_page($mail_campaign) {  
  return theme('mail_campaign_email_admin_page', $mail_campaign);
}

/**
 * Mail Campaign subscriptions administration page. Display a list of subscriptions to this Campaign.
 */
function mail_campaign_subscriptions_admin_page($mail_campaign) {  
  return theme('mail_campaign_subscriptions_admin_page', $mail_campaign);
}

/**
 * Theme the output for the main E-mail Campaigns administration page.
 */
function theme_mail_campaign_admin_page($mail_campaigns) {
  $output = '<p>' . t('This page lists all the <em>E-mail Campaigns</em> that are currently defined on this system. You may <a href="@add-url">add new campaign</a>.', array('@add-url' => url('admin/build/mail_campaign/add'))) . '</p>';
  
  $destination = drupal_get_destination();
  if (empty($mail_campaigns)) {
    $rows[] = array(
      array('data' => t('No campaigns are currently defined.'), 'colspan' => 3),
    );
  }
  else {
    foreach ($mail_campaigns as $mail_campaign) {
    $mail_campaign = mail_campaign_load($mail_campaign->mcid);
    $ops = theme('links', array(
      'mail_campaign_edit' =>  array('title' => t('edit'), 'href' => "admin/build/mail_campaign/" . $mail_campaign->mcid . '/edit'),
      'mail_campaign_delete' =>  array('title' => t('delete'), 'href' => "admin/build/mail_campaign/delete/" . $mail_campaign->mcid),      
    ));
    
    $modes = array(
      MAIL_CAMPAIGN_MODE_INACTIVE => 'Inactive', 
      MAIL_CAMPAIGN_MODE_DEBUG => 'Debug',
      MAIL_CAMPAIGN_MODE_ACTIVE => 'Active'
      );
    
    $rows[] = array(
      l($mail_campaign->campaign_name, 'admin/build/mail_campaign/' . $mail_campaign->mcid . '/view'),
      $modes[$mail_campaign->campaign_mode],
      $ops,
    );
    }
  }
  $header = array(t('Name'), t('Mode'), t('Operations'));
  $output .= theme('table', $header, $rows);
  return $output;
}


/**
 * Theme the output for the main E-mail Campaign email administration page.
 */
function theme_mail_campaign_email_admin_page($mail_campaign) {

  
  $mail_templates = mail_campaign_get_campaign_mails($mail_campaign);
 
  $output .= t(
    '<a href="@add-url">Add email template to campaign</a>',
    array(
        '@add-url' => url(
          'admin/build/mail_campaign/' . arg(3) . '/emails/add'
        )
      )   
     ); 
     
    
  $destination = drupal_get_destination();
  if (empty($mail_templates)) {
    $rows[] = array(
      array('data' => t('No email templates have been added to this campaign.'), 'colspan' => 3),
    );
  }
  else {
    foreach ($mail_templates as $mail) {
      
      $mail_template = db_fetch_object(db_query("SELECT * FROM {mail_template} WHERE mtid=%d", $mail->mtid));      

      $ops = theme('links', array(
        'mail_template_edit' =>  array('title' => t('edit'), 'href' => "admin/build/mail_campaign/" . $mail_campaign->mcid . '/emails/edit/'  . $mail->mcmtid ),
        'mail_campaign_remove' =>  array('title' => t('remove'), 'href' => "admin/build/mail_campaign/" . $mail_campaign->mcid . '/emails/remove/'  . $mail->mcmtid),      
      ));
      
      $rows[] = array(
        $mail_template->title . ' (' . $mail_template->name . ')',
        $mail->send_time,
        $ops,
      );
    }
  }
  $header = array(t('Email template'), t('Send time (days from subscription)'), t('Operations'));
  
  
  $output .= theme('table', $header, $rows);
  return $output;
}

/**
 * Theme the output for the Campaign subscriptions administration page.
 */
function theme_mail_campaign_subscriptions_admin_page($mail_campaign) {
  
  $destination = drupal_get_destination();
  
  $header = array(
    array('data' => t('User'), 'field' => 'name'),
    t('Last mail sent'),
    t('Subscribed'),

    );
  $sql = "SELECT b.name, b.mail, a.current_mail_template_id, a.join_time FROM {mail_campaign_user} a, {users} b where a.uid=b.uid and a.mcid = %d";
  
  $rowcount = 25;
  $query = pager_query($sql . tablesort_sql($header), $rowcount, 0, NULL, $mail_campaign->mcid);    
  
  while ($subscriber = db_fetch_object($query)) {  
    $mail_template = '-';
    if($subscriber->current_mail_template_id > 0)
      $mail_template = db_fetch_result(db_query("SELECT title FROM {mail_template} WHERE mtid=%d", $subscriber->current_mail_template_id));      

    
    $rows[] = array(
      l($subscriber->name, 'user/' . $subscriber->uid),
      $mail_template,
      date('d/m/y',$subscriber->join_time),

    );  
  }
  
  if (empty($rows)) {
    $rows[] = array(
      array('data' => t('No subscriptions for this campaign.'), 'colspan' => 3),
    );
  }
  
  $output .= theme('table', $header, $rows) . theme('pager', $rowcount);
  return $output;
}

/**
 * Add/Edit E-mail Campaign page.
 */
function mail_campaign_add_form(&$form_state, $mail_campaign = NULL) {

  if (!isset($mail_campaign)) {
    drupal_set_title(t('Add new campaign'));
  }
  else {
    // Editing an existing campaign.
    if (empty($mail_campaign)) {
      drupal_goto('admin/build/mail_campaign');
    }
    drupal_set_title(t('Edit (%name) campaign', array('%name' => $mail_campaign->campaign_name)));
  }

  $form['mcid'] = array(
    '#type' => 'value',
    '#value' => $mail_campaign->mcid,
  );
  $form['campaign_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Campaign Name'),
    '#default_value' => $mail_campaign->campaign_name,
    '#description' => t('A short, descriptive title for this email campaign. It will be used in administrative interfaces, and in page titles and menu items.'),
    '#maxlength' => 32,
    '#required' => TRUE,
  );
  $form['campaign_mode'] = array(    
    '#type' => 'select',    
    '#title' => t('Mode'),
    '#options' => array(
      MAIL_CAMPAIGN_MODE_INACTIVE => 'Inactive',
      MAIL_CAMPAIGN_MODE_DEBUG => 'Debug',
      MAIL_CAMPAIGN_MODE_ACTIVE => 'Active'      
      ),
    '#default_value' => $mail_campaign->campaign_mode,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}


/**
 * Validate the Campaign.  
 */
function mail_campaign_add_form_validate($form, &$form_state) {
  mail_campaign_validate_name($form_state['values']['campaign_name'], $form_state);
}

/**
 * Validate the Campaign name.
 */
function mail_campaign_validate_name($campaign_name, $form_state) {
  
  // Ensure the machine name is unique
  if (empty($form_state['values']['mcid'])) {
    $mail_campaign = mail_template_load($campaign_name);
    if ($mail_campaign->campaign_name == $campaign_name) {
      form_set_error('name', t('Campaign names must be unique. This name is already in use.'));
    }
  }
}

/**
 * Update/create an E-mail Campaign.
 */
function mail_campaign_add_form_submit($form, &$form_state) {
  if (empty($form_state['values']['mcid'])) {
    drupal_write_record('mail_campaign', $form_state['values']);
		$mail_campaign = mail_campaign_load($form_state['values']['mcid']);
    drupal_set_message(t('Campaign %name has been added.', array('%name' => $form_state['values']['campaign_name'])));
    watchdog('mail_campaign', 'Campaign %name has been added.', array('%name' => $form_state['values']['campaign_name']), WATCHDOG_NOTICE);
  }
  else {  
    drupal_write_record('mail_campaign', $form_state['values'], 'mcid');
		$mail_campaign = mail_campaign_load($form_state['values']['campaign_name']);
    drupal_set_message(t('Campaign %name has been updated.', array('%name' => $form_state['values']['campaign_name'])));
    watchdog('mail_template', 'Campaign %name has been updated.', array('%name' => $form_state['values']['campaign_name']), WATCHDOG_NOTICE);
  }
  
  $form_state['redirect'] = 'admin/build/mail_campaign';
}

/**
 * Delete E-mail Campaign.
 */
function mail_campaign_delete_confirm(&$form_state, $mcid) {
  
  $mail_campaigns = mail_campaign_load($mcid);
  if (empty($mail_campaigns)) {
    drupal_goto('admin/build/mail_campaign');
  }

  $form['mcid'] = array('#type' => 'value', '#value' => $mail_campaigns->mcid);

  return confirm_form(
    $form,
    t('Are you sure you want to delete %title?', 
    array('%title' => $mail_campaigns->campaign_name)),
    'admin/build/mail_template/campaign',
    t('This action cannot be undone.'),
    t('Delete'), 
    t('Cancel')
  );
}


/**
 * Process campaign delete form submission.
 */
function mail_campaign_delete_confirm_submit($form, &$form_state) {
  $mail_campaigns = mail_campaign_load($form_state['values']['mcid']);
  db_query("DELETE FROM {mail_campaign} WHERE mcid = %d", $mail_campaigns->mcid);
  
  drupal_set_message(t('Campaign %title has been deleted.', 
    array('%title' => $mail_campaigns->campaign_name)));
  watchdog('mail', 'Campaign %title has been deleted.', 
    array('%title' => $mail_campaigns->campaign_name), WATCHDOG_NOTICE);

  $form_state['redirect'] = 'admin/build/mail_campaign';
}


function mail_campaign_remove_email_confirm(&$form_state, $mcmt) {
  
  $mail_template = db_fetch_object(db_query("SELECT * FROM {mail_template} WHERE mtid=%d", $mcmt->mtid));
  
  $mail_campaign = mail_campaign_load($mcmt->mcid);  
  $form['mcmtid'] = array('#type' => 'value', '#value' => $mcmt->mcmtid);

  return confirm_form(
    $form,
    t('Are you sure you want to remove %title from the %campaign?', 
    array('%title' => $mail_template->title, '%campaign' => $mail_campaign->campaign_name)),
    'admin/build/mail_campaign/' . $mcmt->mcid . '/emails',
    '',
    t('Delete'), 
    t('Cancel')
  );
}


/**
 * Process campaign delete form submission.
 */
function mail_campaign_remove_email_confirm_submit($form, &$form_state) {
  $mcmt = mail_campaign_mail_template_load($form_state['values']['mcmtid']);
  db_query("DELETE FROM {mail_campaign_mail_template} WHERE mcmtid = %d", $form_state['values']['mcmtid']);
  
  drupal_set_message(t('Email has been removed.'));

  $form_state['redirect'] = 'admin/build/mail_campaign/' . $mcmt->mcid . '/emails';
}

/**
 * Mail Campaign administration page. Display a list of existing Mail Campaigns.
 */
function mail_campaign_campaign_admin($mail_campaign) {
  drupal_set_title($mail_campaign->campaign_name);
  $modes = array(MAIL_CAMPAIGN_MODE_INACTIVE => 'Inactive', MAIL_CAMPAIGN_MODE_DEBUG => 'Debug', MAIL_CAMPAIGN_MODE_ACTIVE => 'Active');      
  $output = '<b>Mode</b> ' . $modes[$mail_campaign->campaign_mode] .  '<br />';
  if (variable_get('mail_campaign_daily_cron', MAIL_CAMPAIGN_CRONSTAMP) != MAIL_CAMPAIGN_CRONSTAMP){
    $output .= '<b>Cron</b> Cron has not run for today<br />';
  }
  else {
    $output .= '<b>Cron</b> Cron has run for today</b><br />';
  }  
  $mails = db_result(db_query("SELECT count(*) as count FROM {mail_campaign_mail_template} a , {mail_template} b WHERE a.mtid=b.mtid and mcid=%d order by send_time asc", array($mail_campaign->mcid)));      
  $subscribers = db_result(db_query("SELECT count(*) FROM {mail_campaign_user} WHERE mcid=%d", array($mail_campaign->mcid)));      
  $output .= '<b>Mails in campaign:</b> ' . $mails . '<br />';
  $output .= '<b>Subscribers in campaign:</b> ' . $subscribers . '<br />';    
  
  return $output;
}



/**
 * Attach an email to the Campaign.
 */
function mail_campaign_email_add_form(&$form_state, $mail_campaign_template = NULL) {
      
  $mail_campaign = mail_campaign_load(arg(3));
  drupal_set_title(t('Add email template to (%name)', array('%name' => $mail_campaign->campaign_name)));
  
  $mail_templates = mail_template_get_mail_template();
  $mail_template_options = array('0' => '--Select--');
  foreach($mail_templates as $mail) {
    $mail_template_options[$mail->mtid] = $mail->title;
  }  
  
  $form['mtid'] = array(
    '#title' => t('Email template'),
    '#type' => 'select',
    '#required' => true,
    '#options' => $mail_template_options,    
    '#default_value' => $mail_campaign_template->mtid,
  );
  
  $form['send_time'] = array(
    '#title' => t('Send time'),
    '#type' => 'textfield',
    '#required' => true,
    '#field_suffix' => 'Days',
    '#default_value' => $mail_campaign_template->send_time,
    '#size' => 5,
    '#description' => t('Number of days after being subscribed to this campaign that a user will receive this email.'),
  );
  
  $form['mcmtid'] = array('#type' => 'hidden', '#value' => $mail_campaign_template->mcmtid );
  $form['mcid'] = array('#type' => 'hidden', '#value' => $mail_campaign->mcid );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Validate the Campaign.  
 */
function mail_campaign_email_add_form_validate($form, &$form_state) {
  
  if (empty($form_state['values']['mtid']) || $form_state['values']['mtid'] == 0) {
     form_set_error('mcid', t('Please select a valid email template.'));
  }  
  if ($form_state['values']['send_time'] == '' ||  $form_state['values']['send_time'] < 0) {
     form_set_error('send_time', t('Please supply a positive number of days (can be 0).'));
  }  
}

/**
 * Attach an email to the Campaign.
 */
function mail_campaign_email_add_form_submit($form, &$form_state) {
  if (empty($form_state['values']['mcmtid']) ) {
    drupal_write_record('mail_campaign_mail_template', $form_state['values']);
		$mail_campaign = mail_campaign_load($form_state['values']['mcid']);
    drupal_set_message(t('Email template has been added to Campaign %name.', array('%name' => $form_state['values']['campaign_name'])));    
  }
  else {  
    drupal_write_record('mail_campaign_mail_template', $form_state['values'], 'mcmtid');
    drupal_set_message(t('Email template settings updated.'));
  }
  
  $form_state['redirect'] = 'admin/build/mail_campaign/' . $form_state['values']['mcid'] . '/emails';
}