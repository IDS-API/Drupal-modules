<?php

class idsviews_handler_filter_subjects extends views_handler_filter {

  function value_form(&$form, &$form_state) {
    $subjects_values = array('all_subjects' => '--- All Subjects ---');

    if ($this->options['size_select']) {
      $length_title = $this->options['size_select'];
    }
    else {
      $length_title = IDS_VIEWS_DEFAULT_SIZE_CATEGORIES;
    }

    if (isset($this->view->query->request->site)) {
      $dataset = $this->view->query->request->site;
    }
    else {
      $dataset = IDS_API_DEFAULT_DATASET;
    }
    $response = idsapi_get_all('subjects', $dataset, 'full');
    if (isset($response->results)) {
      $subjects = $response->results;
      foreach ($subjects as $subject) {
        $name_subject = substr($subject->title,0,$length_title);
        if (strlen($subject->title) > $length_title) {
          $name_subject .= '...';
        }
        $level = $subject->level;
        $padding_str = '-';
        $padding = strlen($name_subject) + strlen($padding_str)*($level - 1);
        $name_subject = str_pad($name_subject, $padding, $padding_str, STR_PAD_LEFT);
        $subjects_values[$subject->object_id] = $name_subject;
      }
    }
    $form['value'] = array(
      '#type' => 'select',
      '#title'  => t('Subjects'),
      '#multiple' => TRUE,
      '#options'  => $subjects_values,
      '#default_value'  => $this->value,
    );
    if (!empty($form_state['exposed'])) {
      $identifier = $this->options['expose']['identifier'];
      if (!isset($form_state['input'][$identifier])) {
        $form_state['input'][$identifier] = $this->value;
      }
    }
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['size_select'] = array('default' => '');
    return $options;
  }

  function options_form(&$form, &$form_state) {   
    parent::options_form($form, $form_state);
    $form['size_select'] = array(
      '#title' => t('Size'),
      '#type' => 'textfield',
      '#size' => 10,
      '#maxlength' => 10,
      '#description' => t('Number of characters shown for each subject.'),
      '#default_value' => $this->options['size_select'],
    );
  }

  function expose_options() {
    $this->options['expose'] = array(
      'use_operator' => FALSE, 
      'operator' => $this->options['id'] . '_op', 
      'identifier' => $this->options['id'], 
      'label' => $this->definition['title'], 
      'remember' => FALSE, 
      'multiple' => TRUE, 
      'required' => FALSE,
    );
  }

  function query() {
    $group = $this->options['group'];
    $filter = $this->options['field'];
    if (isset($this->value)) {
      if (!isset($this->value['all_subjects'])) {
        $value = implode('|', $this->value);
        if (isset($group, $filter, $value)) {
          if ($value) {
            $this->view->query->add_filter($group, $filter, $value);
          }
        }
      }
    }
  }

}