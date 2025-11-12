#### 测试环境：imx8mp

#### 内核版本：5.15.71

##### 设备树

```
&flexspi {
        pinctrl-names = "default";
        pinctrl-0 = <&pinctrl_flexspi0>;
        status = "okay";
        
        mydriver@0 {
            reg = <0>;
            compatible = "myflexdriver";
            spi-max-frequency = <133000000>;
            spi-rx-bus-width = <4>;
            spi-tx-bus-width = <4>;
        };
};

```



在官方的flexspi驱动代码  **drivers/spi/spi-nxp-fspi.c**  中定义了ahb_buffer (rx) 长度为2K，txfifo (tx) 长度为1K，接收和发送无法超出此长度

```
static struct nxp_fspi_devtype_data imx8mm_data = {
        .rxfifo = SZ_512,       /* (64  * 64 bits)  */
        .txfifo = SZ_1K,        /* (128 * 64 bits)  */
        .ahb_buf_size = SZ_2K,  /* (256 * 64 bits)  */
        .quirks = 0,
        .little_endian = true,  /* little-endian    */
};

static const struct of_device_id nxp_fspi_dt_ids[] = {
        { .compatible = "nxp,lx2160a-fspi", .data = (void *)&lx2160a_data, },
        { .compatible = "nxp,imx8mm-fspi", .data = (void *)&imx8mm_data, },
        { .compatible = "nxp,imx8mp-fspi", .data = (void *)&imx8mm_data, },
        { .compatible = "nxp,imx8qxp-fspi", .data = (void *)&imx8qxp_data, },
        { .compatible = "nxp,imx8dxl-fspi", .data = (void *)&imx8dxl_data, },
        { /* sentinel */ }
};
```
