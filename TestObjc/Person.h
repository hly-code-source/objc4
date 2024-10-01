//
//  Person.h
//  TestObjc
//
//  Created by helinyu on 2024/9/30.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface Person : NSObject

@property (nonatomic, copy) NSString *name;
@property (nonatomic, assign) NSInteger age;

@end

NS_ASSUME_NONNULL_END
