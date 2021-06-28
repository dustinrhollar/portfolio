
using namespace jengine::ecs;

void CreateKeyFrameList(KeyFrameList *key_frame_list)
{
    key_frame_list->Size = 0;
    key_frame_list->Root = nullptr;
}

void CreateKeyFrameIterator(KeyFrameList *key_frame_list, KeyFrameListIterator *iterator)
{
    if (key_frame_list->Root)
        iterator->CurrentKeyFrameNode = &key_frame_list->Root;
    else
        iterator->CurrentKeyFrameNode = nullptr;
    
    iterator->KeyFrameList = key_frame_list;
}

void AddToKeyFrameList(KeyFrameListIterator *iterator,
                       r32 start_time, Channel channel)
{
    //KeyFrameNode *current_node = *(iterator->CurrentKeyFrameNode);
    
    // Empty key frame list
    if (!iterator->CurrentKeyFrameNode)
    {
        KeyFrame frame = {};
        frame.StartTime = start_time;
        frame.Channels = DynamicArray<Channel>(5);
        frame.Channels.PushBack(channel);
        
        iterator->KeyFrameList->Size = 1;
        iterator->KeyFrameList->Root = talloc<KeyFrameNode>(1);
        iterator->KeyFrameList->Root->next = nullptr;
        iterator->KeyFrameList->Root->key_frame = frame;
        
        iterator->CurrentKeyFrameNode = &iterator->KeyFrameList->Root;
    }
    // When adding to the list, we found the current iterator node
    // is less than the passed start_time. Ideally, this only occurs
    // at the root node.
    else if ((*(iterator->CurrentKeyFrameNode))->key_frame.StartTime >
             start_time)
    {
        if (*iterator->CurrentKeyFrameNode != iterator->KeyFrameList->Root)
        {
            printf("Hey, we found a case when adding the a start time was passed that was less than the iterator was not the root.\n");
        }
        
        KeyFrame frame = {};
        frame.StartTime = start_time;
        frame.Channels = DynamicArray<Channel>(5);
        frame.Channels.PushBack(channel);
        
        KeyFrameNode *node = talloc<KeyFrameNode>(1);
        node->next         = iterator->KeyFrameList->Root;
        node->key_frame    = frame;
        
        iterator->KeyFrameList->Root = node;
        iterator->KeyFrameList->Size++;
        iterator->CurrentKeyFrameNode = &iterator->KeyFrameList->Root;
    }
    // Insert to the right or some node to the right
    else
    {
        KeyFrameNode *tmp = *iterator->CurrentKeyFrameNode;
        if (tmp->key_frame.StartTime == start_time)
        {
            tmp->key_frame.Channels.PushBack(channel);
        }
        else
        {
            while (tmp->next && tmp->next->key_frame.StartTime < start_time)
            {
                tmp = tmp->next;
            }
            
            // Two Possible conditions:
            // tmp->next == start_time <- adds to the channel of an existing keyframe
            // tmp->next != start_time <- creates new node in the list
            if (tmp->next && tmp->next->key_frame.StartTime == start_time)
            {
                tmp->next->key_frame.Channels.PushBack(channel);
            }
            else
            {
                KeyFrame frame = {};
                frame.StartTime = start_time;
                frame.Channels = DynamicArray<Channel>(5);
                frame.Channels.PushBack(channel);
                
                KeyFrameNode *node = talloc<KeyFrameNode>(1);
                node->next         = tmp->next;
                node->key_frame    = frame;
                
                tmp->next = node;
                
                iterator->KeyFrameList->Size++;
            }
            
            iterator->CurrentKeyFrameNode = &tmp->next;
        }
    }
}

void CollapseKeyFrameListToArray(KeyFrameList *key_frame_list,
                                 KeyFrame *key_frames)
{
    KeyFrameNode *iterator = key_frame_list->Root;
    for (int i = 0; i < key_frame_list->Size; ++i)
    {
        key_frames[i] = iterator->key_frame;
    }
}

void DestroyKeyFrameList(KeyFrameList *key_frame_list)
{
    KeyFrameNode *iterator = key_frame_list->Root;
    for (int i = 0; i < key_frame_list->Size; ++i)
    {
        iterator->key_frame.Channels.~DynamicArray();
    }
    
    key_frame_list->Root = nullptr;
}


void AnimationSystem::Update()
{
    // What to do?
    ComponentIter<AnimationComponent> fox_iter = GetComponentIter<AnimationComponent>();
    
    AnimationComponent *animation = nullptr;
    while ((animation = fox_iter.next()))
    {
        printf("Hey, an animation was found: %s!\n", animation->ActiveAnimation->Name.GetCStr());
        RemoveEntityFromComponent<AnimationComponent>(animation->entity);
    }
}