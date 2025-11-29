#ifndef ENTITYSTATE_H
#define ENTITYSTATE_H

/**
 * @brief 实体状态枚举
 * 用于跟踪实体的持久化状态，实现增量保存
 * 遵循标准的ORM状态管理模式
 */
enum class EntityState
{
    /**
     * @brief 分离态
     * 实体未被上下文跟踪，不会被持久化
     * 通常用于从外部加载或临时创建的实体
     */
    Detached = 0,
    
    /**
     * @brief 未变更态
     * 实体已从数据库加载，且未被修改
     * 保存时跳过此类实体
     */
    Unchanged = 1,
    
    /**
     * @brief 新增态
     * 实体是新创建的，尚未插入数据库
     * 保存时执行 INSERT 操作
     */
    Added = 2,
    
    /**
     * @brief 修改态
     * 实体已从数据库加载，但已被修改
     * 保存时执行 UPDATE 操作
     */
    Modified = 3,
    
    /**
     * @brief 删除态
     * 实体已从数据库加载，但已被标记为删除
     * 保存时执行 DELETE 操作
     */
    Deleted = 4
};

/**
 * @brief 获取状态的显示名称
 */
inline QString entityStateToString(EntityState state)
{
    switch (state) {
        case EntityState::Detached:  return "Detached";
        case EntityState::Unchanged: return "Unchanged";
        case EntityState::Added:     return "Added";
        case EntityState::Modified:  return "Modified";
        case EntityState::Deleted:   return "Deleted";
        default:                     return "Unknown";
    }
}

#endif // ENTITYSTATE_H

