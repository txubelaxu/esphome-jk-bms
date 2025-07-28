

uint8_t JkRS485Sniffer::manage_rx_buffer_initial(void) {

  const uint8_t *raw = &this->rx_buffer_[0];
  uint8_t address = 0;

  const uint32_t now = millis();

  /*
  const size_t free_heap = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  ESP_LOGV(TAG, "free_heap %f kBytes [buffer: %d bytes]",((float)free_heap/1024),this->rx_buffer_.size());
  */
  if (this->rx_buffer_.size()>=JKPB_RS485_MASTER_SHORT_REQUEST_SIZE){
    auto it = std::search(this->rx_buffer_.begin(), this->rx_buffer_.end(), pattern_response_header.begin(), pattern_response_header.end());
    if (it == this->rx_buffer_.end()) {
      // Start sequence NOT FOUND (0x55AAEB90) --> maybe short response to a real master request?
      // no squence
      uint16_t computed_checksum = crc16_c(raw, 6); 
      uint16_t remote_checksum = ((uint16_t(raw[6]) << 8) | (uint16_t(raw[7]) << 0) );

      if (computed_checksum != remote_checksum) {
        ESP_LOGV(TAG, "CHECKSUM failed! 0x%04X != 0x%04X", computed_checksum, remote_checksum);
        //IT IS NOT A SHORT REQUEST OR THERE WAS A COMM. ERROR --> continue whith manage_rx_buffer code
      } else {
        address=raw[0];
        //ESP_LOGI(TAG, "REAL master is speaking to address 0x%02X (short request)",address);

        //this->rs485_network_node[0].last_message_received=now;
        //this->detected_master_activity_now();
        
        //this->set_node_availability(0,1);
        std::vector<uint8_t> data(this->rx_buffer_.begin() + 0, this->rx_buffer_.begin() + JKPB_RS485_MASTER_SHORT_REQUEST_SIZE-1);
        ESP_LOGD(TAG, "Answer received for MASTER (type: SHORT REQUEST for address %02X, %d bytes)",address, data.size());
        this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + JKPB_RS485_MASTER_SHORT_REQUEST_SIZE-1); 
        //continue with next;
        return(7);
      }
    }
  }

  if (this->rx_buffer_.size()>=JKPB_RS485_MASTER_REQUEST_SIZE){
    auto it = std::search(this->rx_buffer_.begin(), this->rx_buffer_.end(), pattern_response_header.begin(), pattern_response_header.end());
    bool try_with_master_request_size=false;
    if (it == this->rx_buffer_.end()) {
      //no sequence
      try_with_master_request_size=true;
    } else {
      //sequence found, but where?
      size_t index = std::distance(this->rx_buffer_.begin(), it);
      if (index>=JKPB_RS485_MASTER_REQUEST_SIZE){
        try_with_master_request_size=true;
      }      
    }

    if (try_with_master_request_size==true){
      // Start sequence NOT FOUND (0x55AAEB90) --> maybe short response to a real master request?

      uint16_t computed_checksum = crc16_c(raw, 9); 
      uint16_t remote_checksum = ((uint16_t(raw[9]) << 8) | (uint16_t(raw[10]) << 0) );

      if (computed_checksum != remote_checksum) {
        ESP_LOGV(TAG, "CHECKSUM failed! 0x%04X != 0x%04X", computed_checksum, remote_checksum);
        //NO, OR THERE WAS A COMM. ERROR
      } else {
        address=raw[0];
        ESP_LOGI(TAG, "REAL master is speaking to address 0x%02X (request)",address);
        this->rs485_network_node[0].last_message_received=now;
        this->detected_master_activity_now();
        this->set_node_availability(0,1);
        //std::vector<uint8_t> data(this->rx_buffer_.begin() + 0, this->rx_buffer_.begin() + JKPB_RS485_MASTER_REQUEST_SIZE-1);
        //ESP_LOGD(TAG, "Frame received from MASTER (type: REQUEST for address %02X, %d bytes)",address, data.size());
        this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + JKPB_RS485_MASTER_REQUEST_SIZE); 
        //continue with next;
        return(6);
      }
    }
  }

  if (this->rx_buffer_.size()>=JKPB_RS485_RESPONSE_SIZE){
    auto it = std::search(this->rx_buffer_.begin(), this->rx_buffer_.end(), pattern_response_header.begin(), pattern_response_header.end());

    if (it != this->rx_buffer_.end()) {
      //Sequence found, but where?

      size_t index = std::distance(this->rx_buffer_.begin(), it);
      
      if (index>0){
        //printBuffer(index);
        this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + index);    
        //continue with next;            
      }

      if (this->rx_buffer_.size()>=JKPB_RS485_RESPONSE_SIZE){
        //continue
        ESP_LOGD(TAG, "###############################Sequence found SIZE: %d",(this->rx_buffer_.size()));     
      } else {
        return(3);
      }
    } else {
      return(4);
    }  
  } else {
    return(5);
  }


  // Start sequence (0x55AAEB90) //55aaeb90 0105

  if (this->rx_buffer_.size() >= JKPB_RS485_RESPONSE_SIZE){
    uint8_t computed_checksum = chksum(raw, JKPB_RS485_NUMBER_OF_ELEMENTS_TO_COMPUTE_CHECKSUM);
    uint8_t remote_checksum = raw[JKPB_RS485_CHECKSUM_INDEX];

    if (raw[JKPB_RS485_FRAME_TYPE_ADDRESS]==1){
      address=raw[JKPB_RS485_FRAME_TYPE_ADDRESS_FOR_FRAME_TYPE_x01+6];
    } else {
      address=raw[JKPB_RS485_ADDRESS_OF_RS485_ADDRESS];
    }
    //ESP_LOGI(TAG, "(this->rx_buffer_.size():%03d) [address 0x%02X] Frame Type 0x%02X ",this->rx_buffer_.size(),address,raw[JKPB_RS485_FRAME_TYPE_ADDRESS]);

    if (computed_checksum != remote_checksum) {
      ESP_LOGW(TAG, "CHECKSUM failed! 0x%02X != 0x%02X", computed_checksum, remote_checksum);
      auto it_next = std::search(this->rx_buffer_.begin()+1, this->rx_buffer_.end(), pattern_response_header.begin(), pattern_response_header.end());
      size_t index_next = std::distance(this->rx_buffer_.begin(), it_next);
      
      if (index_next>0){
        //printBuffer(index);
        this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + index_next);        
      } else {
          this->rx_buffer_.clear();
      }

      return(10);
    } else {
      this->rs485_network_node[address].last_message_received=now;
      if (address==0){
        last_master_activity=now;
      } else if (address>15){
        ESP_LOGV(TAG, "(this->rx_buffer_.size():%03d) [address 0x%02X] Frame Type 0x%02X | CHECKSUM is correct",this->rx_buffer_.size(),address,raw[JKPB_RS485_FRAME_TYPE_ADDRESS]);
        //printBuffer(0);
        this->rx_buffer_.clear();
        return(11);
      } else {
        this->set_node_availability(address,1);
      }
      
    }





    std::vector<uint8_t> data(this->rx_buffer_.begin() + 0, this->rx_buffer_.begin() + this->rx_buffer_.size()+1);
    
    ESP_LOGD(TAG, "Frame received from SLAVE (type: 0x%02X, %d bytes) %02X address", raw[4], data.size(),address);
    ESP_LOGVV(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

    ESP_LOGD(TAG, "1");

    bool found = false;
    for (auto *device : this->devices_) {
        ESP_LOGD(TAG, "2");
        device->on_jk_rs485_sniffer_data(address, raw[JKPB_RS485_FRAME_TYPE_ADDRESS], data, this->nodes_available );   
        found = true;
    }
    
    if (!found) {
      ESP_LOGW(TAG, "Got JkRS485 but no recipients to send [frame type:0x%02X] 0x%02X!",raw[JKPB_RS485_FRAME_TYPE_ADDRESS], address);
    }
  } else {
    //    ESP_LOGD(TAG, "rx_buffer_.size=%02d",this->rx_buffer_.size()); 
  }

  this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + JKPB_RS485_RESPONSE_SIZE);
  return(12);
}


uint8_t JkRS485Sniffer::manage_rx_buffer_20250727(void) {

  const uint8_t *raw = &this->rx_buffer_[0];
  uint8_t address = 0;
  bool frame_well_formed = false;

  bool try_with_master_request_size = false;

  const uint32_t now = millis();

  ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-->");
  ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-address: %02X ", address);
  ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-frame_well_formed: %d ", frame_well_formed);
  
  /*
  const size_t free_heap = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  ESP_LOGV(TAG, "free_heap %f kBytes [buffer: %d bytes]",((float)free_heap/1024),this->rx_buffer_.size());
  */

  ESP_LOGV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-[buffer: %d bytes]",this->rx_buffer_.size());

  if (this->rx_buffer_.size() >= JKPB_RS485_MASTER_SHORT_REQUEST_SIZE) {
    ESP_LOGV(TAG, "JkRS485Sniffer::manage_rx_buffer_()- >=[JKPB_RS485_MASTER_SHORT_REQUEST_SIZE: %d bytes]",JKPB_RS485_MASTER_SHORT_REQUEST_SIZE);

    auto it = std::search(this->rx_buffer_.begin(), this->rx_buffer_.end(), pattern_response_header.begin(), pattern_response_header.end());

    if (it == this->rx_buffer_.end()) {
      // Start sequence NOT FOUND (0x55AAEB90) --> maybe short response to a real master request?      
      // no squence 
      ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-No start sequence found. (0x55AAEB90) ");                        

      uint16_t computed_checksum = crc16_c(raw, 6);
      uint16_t remote_checksum = ((uint16_t(raw[6]) << 8) | (uint16_t(raw[7]) << 0));

      if (computed_checksum != remote_checksum) {
        ESP_LOGV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_MASTER_SHORT_REQUEST_SIZE-CHECKSUM failed! 0x%04X != 0x%04X", computed_checksum, remote_checksum);
        // IT IS NOT A SHORT REQUEST OR THERE WAS A COMM. ERROR --> continue whith manage_rx_buffer code
      } else {
        address = raw[0];
        ESP_LOGVV(TAG, "REAL master is speaking to address 0x%02X (short request)",address);

        // this->rs485_network_node[0].last_message_received=now;
        // this->detected_master_activity_now();

        // this->set_node_availability(0,1);
        std::vector<uint8_t> data(this->rx_buffer_.begin() + 0, this->rx_buffer_.begin() + JKPB_RS485_MASTER_SHORT_REQUEST_SIZE - 1);
        ESP_LOGD(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_MASTER_SHORT_REQUEST_SIZE-Answer received for MASTER (type: SHORT REQUEST for address %02X, %d bytes)", address, data.size());
                 
        this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + JKPB_RS485_MASTER_SHORT_REQUEST_SIZE - 1);

        //After deletion the variable should be assigned again
        const uint8_t *raw = &this->rx_buffer_[0];

        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_MASTER_SHORT_REQUEST_SIZE-.........................................................");                        
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_MASTER_SHORT_REQUEST_SIZE-AFTER ERASE: [buffer: %d bytes]",this->rx_buffer_.size());                        
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_MASTER_SHORT_REQUEST_SIZE-AFTER ERASE: %s", format_hex_pretty(&this->rx_buffer_.front(), this->rx_buffer_.size()).c_str());
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_MASTER_SHORT_REQUEST_SIZE-.........................................................");                        
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_MASTER_SHORT_REQUEST_SIZE-Return 7 ??¿¿??¿");
        // continue with next;
        return (BUFFER_RESPONSE_NO_START_SEQUENCE);
      }
    }
    else{
      ESP_LOGV(TAG, "JkRS485Sniffer::manage_rx_buffer_()- >=[JKPB_RS485_MASTER_SHORT_REQUEST_SIZE: buffer is not at the end or empty");
    }
  }

  if (this->rx_buffer_.size() >= JKPB_RS485_MASTER_REQUEST_SIZE) {
    ESP_LOGV(TAG, "JkRS485Sniffer::manage_rx_buffer_()- >=[JKPB_RS485_MASTER_REQUEST_SIZE: %d bytes]",JKPB_RS485_MASTER_REQUEST_SIZE);    
    
    auto it = std::search(this->rx_buffer_.begin(), this->rx_buffer_.end(), pattern_response_header.begin(), pattern_response_header.end());

    //bool try_with_master_request_size = false;

    if (it == this->rx_buffer_.end()) {
      // no sequence
      try_with_master_request_size = true;

    } else {
      // sequence found, but where?
      size_t index = std::distance(this->rx_buffer_.begin(), it);

      ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-[JKPB_RS485_MASTER_REQUEST_SIZE - START SEQUENCE FOUND AT POSITION: %d ]",index);          

      if (index >= JKPB_RS485_MASTER_REQUEST_SIZE) {
        try_with_master_request_size = true;
      }

    }

    if (try_with_master_request_size == true) {
      ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-[JKPB_RS485_MASTER_REQUEST_SIZE]- START SEQUENCE FOUND WITHING THE FRAME try_with_master_request_size == true");          

      uint16_t computed_checksum = crc16_c(raw, 9);
      uint16_t remote_checksum = ((uint16_t(raw[9]) << 8) | (uint16_t(raw[10]) << 0));

      if (computed_checksum != remote_checksum) {
        ESP_LOGV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_MASTER_REQUEST_SIZE-CHECKSUM failed! 0x%04X != 0x%04X", computed_checksum, remote_checksum);
        // NO, OR THERE WAS A COMM. ERROR
      } else {

        address = raw[0];

        ESP_LOGI(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_MASTER_REQUEST_SIZE-REAL master is speaking to address 0x%02X (request)", address);

        this->rs485_network_node[0].last_message_received = now;
        this->detected_master_activity_now();
        this->set_node_availability(0, 1);

        //2025-07-25/01-rabbit: Avoid deleting the header of the frame with the address.
        // this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + JKPB_RS485_MASTER_REQUEST_SIZE);
        // //After deletion the variable should be assigned again
        // *raw = &this->rx_buffer_[0];        

        // ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_MASTER_REQUEST_SIZE-.........................................................");                        
        // ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_MASTER_REQUEST_SIZE-AFTER ERASE: [buffer: %d bytes]",this->rx_buffer_.size());                        
        // ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_MASTER_REQUEST_SIZE-AFTER ERASE: %s", format_hex_pretty(&this->rx_buffer_.front(), this->rx_buffer_.size()).c_str());
        // ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_MASTER_REQUEST_SIZE-.........................................................");                        

        // ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_MASTER_REQUEST_SIZE-Return 6 ??¿¿??¿");
        // // continue with next;
        // return (6);

        //2025-07-25-rabbit: Avoid deleting the header of the frame with the address^^^^^^^^^^^^.

      }
    }
  }

  if (this->rx_buffer_.size() >= JKPB_RS485_RESPONSE_SIZE) {
    ESP_LOGV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-[buffer: %d bytes]>=[JKPB_RS485_RESPONSE_SIZE: %d bytes]",this->rx_buffer_.size(),JKPB_RS485_RESPONSE_SIZE);    

    auto it = std::search(this->rx_buffer_.begin(), this->rx_buffer_.end(), pattern_response_header.begin(), pattern_response_header.end());

    if (it != this->rx_buffer_.end()) {
      // Sequence found, but where?

      address = raw[0];     

      ESP_LOGI(TAG, "JkRS485Sniffer::manage_rx_buffer_()-[buffer]>=[JKPB_RS485_RESPONSE_SIZE] - address [0x%02X]", address);

      size_t index = std::distance(this->rx_buffer_.begin(), it);

      if (index > 0) {
        // printBuffer(index);
        
        //2025-07-25-rabbit: Delete the previous info except the address section. 
        // this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + index);        
        //2025-07-25-rabbit: This will maintain the frame with the address.
        this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + index - JKPB_RS485_MASTER_REQUEST_SIZE);        
        //After deletion the variable should be assigned again
        const uint8_t *raw = &this->rx_buffer_[0];
        //Get the real address
        address = raw[0];
       
        // continue with next;
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE-.........................................................");                        
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE-AFTER ERASE: new address: [0x%02X]", address );                        
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE-AFTER ERASE: [buffer: %d bytes]",this->rx_buffer_.size());                        
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE-AFTER ERASE: %s", format_hex_pretty(&this->rx_buffer_.front(), this->rx_buffer_.size()).c_str());
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE-.........................................................");                        
        //ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_MASTER_REQUEST_SIZE-Return 6 ??¿¿??¿");        
      }

      if (this->rx_buffer_.size() >= JKPB_RS485_RESPONSE_SIZE) {
        // continue
        ESP_LOGD(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE-###############################Sequence found SIZE: %d", (this->rx_buffer_.size()));
        frame_well_formed = true;
      } else {
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE-Return 3 ??¿¿??¿");        
        return (BUFFER_RESPONSE_BUFFER_SIZE_LESS_THAN_RESPONSE_SIZE_AFTER_ERASE);
      }
    } else {

      //if (it != this->rx_buffer_.end()) {
      ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE-Return 4 ??¿¿??¿");        

      return (4);
    }
  } else {
    //if (this->rx_buffer_.size() >= JKPB_RS485_RESPONSE_SIZE) {    
    ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-[buffer: %d bytes] < [JKPB_RS485_RESPONSE_SIZE: %d bytes]",this->rx_buffer_.size(),JKPB_RS485_RESPONSE_SIZE);    
    ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE-Return 5 == BUFFER_RESPONSE_BUFFER_SIZE_LESS_THAN_RESPONSE_SIZE ");        
    return (BUFFER_RESPONSE_BUFFER_SIZE_LESS_THAN_RESPONSE_SIZE);
  }

  // Start sequence (0x55AAEB90) //55aaeb90 0105

  if (this->rx_buffer_.size() >= JKPB_RS485_RESPONSE_SIZE) {
    ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE 02-this->rx_buffer_.size() >= JKPB_RS485_RESPONSE_SIZE");
    
    //2025-07-25-rabbit: the address is supossed to be extract in the previous 'if', so now we can delete the address section
    auto it = std::search(this->rx_buffer_.begin(), this->rx_buffer_.end(), pattern_response_header.begin(), pattern_response_header.end());
    size_t index = std::distance(this->rx_buffer_.begin(), it);
    this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + index);        
    //After deletion the variable should be assigned again
    const uint8_t *raw = &this->rx_buffer_[0];

        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-[JKPB_RS485_MASTER_REQUEST_SIZE - START SEQUENCE FOUND AT POSITION: %d ]",index);          
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE 02-.........................................................");                        
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE 02-AFTER ERASE: [buffer: %d bytes]",this->rx_buffer_.size());                        
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE 02-AFTER ERASE: %s", format_hex_pretty(&this->rx_buffer_.front(), this->rx_buffer_.size()).c_str());
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE 02-.........................................................");     

    /*
    2025-07-25-rabbit: For firmwares >= 15.38 the master is comming in the 0F 
      and the frames 01 and 02 are comming together without the address section.
      the computed checksum should be different in this situation.
      Lets make a workarround to avoid the checksum and to have again the address 0.
    */    
    
    uint8_t computed_checksum = chksum(raw, JKPB_RS485_NUMBER_OF_ELEMENTS_TO_COMPUTE_CHECKSUM);
    uint8_t remote_checksum = raw[JKPB_RS485_CHECKSUM_INDEX];

    //Lets make a workarround to avoid the checksum and to have again the address 0.
    if (index ==0)
    {
      computed_checksum = remote_checksum;
      if( old_address == 0 && address > 15)
      {
        address = 0;
        old_address = -1;
      }

    }

    
    // Define the start and end positions for the new variable
    // For example, if you want to start from index 4 and go up to JKPB_RS485_NUMBER_OF_ELEMENTS_TO_COMPUTE_CHECKSUM
    // int indexPos = 0;
    // const uint8_t *data_for_checksum = &raw[indexPos]; // Starting from index 0
    // uint16_t length_for_checksum = JKPB_RS485_NUMBER_OF_ELEMENTS_TO_COMPUTE_CHECKSUM - indexPos; // Adjust length if starting offset

    // uint8_t computed_checksum = chksum(data_for_checksum, length_for_checksum);
    // uint8_t remote_checksum = raw[JKPB_RS485_CHECKSUM_INDEX];

    //2025-0725-rabbit: This is a mess, is mixing the address for the device with the frame type
    // if (raw[JKPB_RS485_FRAME_TYPE_ADDRESS] == 1) {
    //   address = raw[JKPB_RS485_FRAME_TYPE_ADDRESS_FOR_FRAME_TYPE_x01 + 6];
    //   ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()- IF Change address to: [0x%02X]", address);
    // } else {
    //   address = raw[JKPB_RS485_ADDRESS_OF_RS485_ADDRESS];
    //   ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()- ELSE Change address to: [0x%02X]", address);
    // }
    // ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-(this->rx_buffer_.size():%03d) [address 0x%02X] Frame Type 0x%02X ", this->rx_buffer_.size(), address, raw[JKPB_RS485_FRAME_TYPE_ADDRESS]);

    if (computed_checksum != remote_checksum) {

      ESP_LOGW(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE 02-CHECKSUM failed! 0x%02X != 0x%02X", computed_checksum, remote_checksum);

      auto it_next = std::search(this->rx_buffer_.begin() + 1, this->rx_buffer_.end(), pattern_response_header.begin(), pattern_response_header.end());
      size_t index_next = std::distance(this->rx_buffer_.begin(), it_next);

      if (index_next > 0) {
        // printBuffer(index);
        this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + index_next);
      } else {
        this->rx_buffer_.clear();
      }
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE 02-Return 10 ??¿¿??¿");        
      return (10);

    } else {

      ESP_LOGW(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE 02-CHECKSUM OK! 0x%02X == 0x%02X", computed_checksum, remote_checksum);      

      this->rs485_network_node[address].last_message_received = now;

      if (address == 0) {
        last_master_activity = now;
      } 
      else if (address > 15) {
        ESP_LOGV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE 02-else if (address > 15)");
        // printBuffer(0);
        this->rx_buffer_.clear();
        ESP_LOGV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE 02-Buffer emptied");
        ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE 02-Return 11 ??¿¿??¿");        
        return (BUFFER_RESPONSE_JK_BMS_ADDRESS_GREATER_15);

      } 
      else {
        this->set_node_availability(address, 1);
      }
    }

    //DO NOT GET THE FULL BUFFER.
    // std::vector<uint8_t> data(this->rx_buffer_.begin() + 0, this->rx_buffer_.begin() + this->rx_buffer_.size() + 1);
    //GET ONLY THE LENGTH OF JKPB_RS485_RESPONSE_SIZE(308)
    std::vector<uint8_t> data(this->rx_buffer_.begin() + 0, this->rx_buffer_.begin() + JKPB_RS485_RESPONSE_SIZE + 1);
    //DELETE THE FRAME THAT IS GOING TO BE PROCESSED.
    // this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + JKPB_RS485_RESPONSE_SIZE);

    //2025-07-26-rabbit: Workarround for master address that is comming at 0F(15). 
    //Add a variable to the esphome config yaml, to force the user to choose the firmware version.
    if (try_with_master_request_size && address==15){
        address = 0;
        old_address = address;
    }

    ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-FOUND FRAME.........................................................");                        
    ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-FOUND FRAME: [buffer: %d bytes]",this->rx_buffer_.size());                        
    ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-FOUND FRAME: %s", format_hex_pretty(&this->rx_buffer_.front(), this->rx_buffer_.size()).c_str());
    ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-FOUND FRAME-.........................................................");     

    ESP_LOGD(TAG, "JkRS485Sniffer::manage_rx_buffer_()-Frame received from SLAVE (type: 0x%02X, %d bytes) %02X address", raw[4], data.size(), address);
    ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-  %s", format_hex_pretty(&data.front(), data.size()).c_str());

    bool found = false;

    for (auto *device : this->devices_) {

      ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JkRS485Sniffer::on_jk_rs485_sniffer_data()-->*************************************************************************");
      ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JkRS485Sniffer::on_jk_rs485_sniffer_data()-->*************************************************************************");
      ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JkRS485Sniffer::manage_rx_buffer_()->on_jk_rs485_sniffer_data()");
      ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JkRS485Sniffer::manage_rx_buffer_()->on_jk_rs485_sniffer_data(%d, %d)", address, raw[JKPB_RS485_FRAME_TYPE_ADDRESS]);
      ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JkRS485Sniffer::on_jk_rs485_sniffer_data()-->*************************************************************************");      
      ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JkRS485Sniffer::on_jk_rs485_sniffer_data()-->*************************************************************************");      

      device->on_jk_rs485_sniffer_data(address, raw[JKPB_RS485_FRAME_TYPE_ADDRESS], data, this->nodes_available);

      found = true;

    }

    ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-->on_jk_rs485_sniffer_data(): found = %d",found);

    if (!found) {
      ESP_LOGW(TAG, "JkRS485Sniffer::manage_rx_buffer_()-Got JkRS485 but no recipients to send [frame type:0x%02X] 0x%02X!", raw[JKPB_RS485_FRAME_TYPE_ADDRESS], address);
    }


  } else {
    //    ESP_LOGD(TAG, "rx_buffer_.size=%02d",this->rx_buffer_.size());
  }

  //DELETE THE FRAME THAT IS ALREADY PROCESSED.
  this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + JKPB_RS485_RESPONSE_SIZE);

  ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()-JKPB_RS485_RESPONSE_SIZE-Return 12 ??¿¿??¿");
  ESP_LOGVV(TAG, "JkRS485Sniffer::manage_rx_buffer_()--<"); 
  return (BUFFER_RESPONSE_FRAME_PROCESSED);
}


