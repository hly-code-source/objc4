//
//  main.m
//  TestObjc
//
//  Created by helinyu on 2024/9/30.
//

#import <Foundation/Foundation.h>
#import "Person.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        
        Person *p = [Person new];
        p.name = @"helinyu";
        p.age = 18;
        NSLog(@"person name :%@, age: %ld", p.name, (long)p.age);
        // insert code here..
    }
    return 0;
}
