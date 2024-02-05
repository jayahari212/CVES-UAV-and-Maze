#include <stdio.h>
#include <stdlib.h>
#include <rcl/rcl.h> //allows us to develop ros2 applications in C (https://docs.ros2.org/latest/api/rcl/)
#include <geometry_msgs/msg/twist.h>

rcl_publisher_t velocity_publisher;

void movement_callback(const void *msgin) { //called when new message is received
    //navigation logic based on received messages
}

int main(int argc, char *argv[]){
    //initialize publisher 
    //publish message to cmd_vel topic to tell drone to fly forward w linear velocity
    // need to adjust velocity value from 1.0- what should we choose?
    rcl_init_options_t options = rcl_get_zero_initialized_init_options();
    rcl_context_t context = rcl_get_zero_initialized_context();
    rcl_init_options_init(&options, rcl_get_default_allocator());
    rcl_init(argc, argv, &options, &context);

    rcl_node_t my_node = rcl_get_zero_initialized_node();
    rcl_node_options_t node_ops = rcl_node_get_default_options();
    rcl_node_init(&my_node, "movement_node", "", &context, &node_ops); //initialize ROS node

    rcl_subscription_t my_subscription = rcl_get_zero_initialized_subscription();
    rcl_subscription_options_t subscription_ops = rcl_subscription_get_default_options();
    rcl_subscription_init(&my_subscription, &my_node, ROSIDL_GET_MSG_TYPE_SUPPORT(/*message type?????*/), "/maze_topic", &subscription_ops);

    //publisher to control drone velocity
    velocity_publisher = rcl_get_zero_initialized_publisher();
    rcl_publisher_options_t publisher_ops = rcl_publisher_get_default_options();
    rcl_publisher_init(&velocity_publisher, &my_node, ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs/msg/Twist), "/cmd_vel", &publisher_ops);

    //insert maze navigation logic here (subscribe to topics, initialize variables, etc)

    rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set(); //wait for data on subscribed topic
    rcl_wait_set_init(&wait_set, 1, 0, 0, 0, 0, rcl_get_default_allocator());

    while (rcl_ok()) {
        // Set up wait set for subscription
        rcl_ret_t ret = rcl_wait_set_clear(&wait_set); 
        ret = rcl_wait_set_add_subscription(&wait_set, &my_subscription, NULL);
        ret = rcl_wait(&wait_set, RCL_MS_TO_NS(100));
        
        if (ret == RCL_RET_TIMEOUT) {
            //nonew messages received within the timeout
            continue;
        }

        //handle received messages
        maze_navigation_callback(/*pass received message data */); //replace comment once we know what message data will be!!!
        
        //telling px4 drone to fly forward. 
        //see twist doc: https://docs.ros.org/en/melodic/api/geometry_msgs/html/msg/Twist.html
        //see ros2 tutorial with twist: https://docs.ros.org/en/eloquent/Tutorials/Topics/Understanding-ROS2-Topics.html
        geometry_msgs__msg__Twist twist_msg;
        geometry_msgs__msg__Twist__init(&twist_msg);
        twist_msg.linear.x = 1.0; //setting forward velocity
        rcl_publish(&velocity_publisher, &twist_msg, NULL);
        geometry_msgs__msg__Twist__fini(&twist_msg);
        //or maybe use this instead to control speed? https://discuss.px4.io/t/offboard-control-using-ros2-how-to-achieve-velocity-control/21875/7
        //it may be best to use c++ for the first solution bc there's examples on the user guide for cpp
        //https://docs.px4.io/main/en/ros/ros2_offboard_control.html
    }

    rcl_subscription_fini(&my_subscription, &my_node);
    rcl_publisher_fini(&velocity_publisher, &my_node);
    rcl_node_fini(&my_node);
    rcl_shutdown(&context);

    return 0;
}
