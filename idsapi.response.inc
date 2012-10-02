<?php

/**
 * IDS API Response class.
 *
 * Objects of this class will be created for responses to API calls.
 *
 * Use IdsApiRequest::factory(my.methodname) to get a response object.
 * All public methods return $this and can be chained together.
 */

// TODO Add interface, etc.

class IdsApiResponse {

  // Results. Array of IdsApiObject (or its subclasses).
  public $results = array();

  // Total results. Number of total available results in the collection.
  public $total_results;

  /**
   * Constructor.
   * 
   * Receives an array with the decoded output of the API call.
   * Is called by IdsApiRequest->makeRequest().
   */
  function __construct($results, $format, $type_results, $total_results, $default_site) {
    foreach ($results as $object) {
      if (!isset($object['site'])) {
        $object['site'] = $default_site;
      }
      $ids_api_object = IdsApiObject::factory($object, $format, $type_results, $default_site);
      array_push($this->results, $ids_api_object);
    }
    if (isset($total_results)) {
      $this->total_results = $total_results;
    }
  }

  /**
   * Creates an array with the short responses formatted to be printed.
   * 
   */
  function htmlShortListObjects() {
    foreach ($this->results as $object) {
      $list_objects[] = l($object->title, htmlspecialchars_decode($object->website_url), array('attributes' => array('class' => array(IDS_CSS_OBJECT))));
    }
    return $list_objects;
  }
}










  
