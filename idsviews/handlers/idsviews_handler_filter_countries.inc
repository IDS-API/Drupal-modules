<?php

class idsviews_handler_filter_countries extends views_handler_filter {

  function value_form(&$form, &$form_state) {
    $countries_values = array('all_countries' => '--- All countries ---');
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
      $dataset = IDS_DEFAULT_DATASET;
    }
    $response = idsapi_get_all('countries', $dataset, 'short');
    if (isset($response->results)) {
      $countries = $response->results;
      foreach ($countries as $country) {
        $name_country = substr($country->title,0,$length_title);
        if (strlen($country->title) > $length_title) {
          $name_country .= '...';
        }
        $countries_values[$country->object_id] = $name_country;
      }
    }
    $form['value'] = array(
      '#type' => 'select',
      '#title'  => t('Countries'),
      '#multiple' => TRUE,
      '#options'  => $countries_values,
      '#default_value'  => $countries_values,
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
      '#description' => t('Number of characters shown for each country.'),
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
      if (!isset($this->value['all_countries'])) {
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